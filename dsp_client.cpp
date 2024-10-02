#include "dsp_client.h"

#include <cstring>

static std::unordered_map<std::string, double> notas = {
    {"do3", 130.8127827},
    {"do3#", 138.5913155},
    {"re3", 146.832384},
    {"re3#", 155.5634919},
    {"mi3", 164.8137785},
    {"fa3", 174.6141157},
    {"fa3#", 184.9972114},
    {"sol3", 195.997718},
    {"sol3#", 207.6523488},
    {"la3", 220},
    {"la3#", 233.0818808},
    {"si3", 246.9416506},

    {"do4", 261.6255653},
    {"do4#", 277.182631},
    {"re4", 293.6647679},
    {"re4#", 311.1269837},
    {"mi4", 329.6275569},
    {"fa4", 349.2282314},
    {"fa4#", 369.9944227},
    {"sol4", 391.995436},
    {"sol4#", 415.3046976},
    {"la4", 440},
    {"la4#", 466.1637615},
    {"si4", 493.8833013},

    {"do5", 523.2511306},
    {"do5#", 554.365262},
    {"re5", 587.3295358},
    {"re5#", 622.2539674},
    {"mi5", 659.2551138},
    {"fa5", 698.4564629},
    {"fa5#", 739.9888454},
    {"sol5", 783.990872},
    {"sol5#", 830.6093952},
    {"la5", 880},
    {"la5#", 932.327523},
    {"si5", 987.7666025}};

dsp_client::dsp_client() : current_mode(Mode::Passthrough), volume(1.0), energy_window_size(0.5), energy_mode(false), period_mode(false), period_minfreq(60.0), period_maxfreq(600.0), period_minlevel(0.5), period_window_size(0.5), period_ringsize(0.5), period(-1), second_period(-1), freq_tuned(-1), note_tuned(""), frequency_difference(0.5) {}

dsp_client::~dsp_client() {}

jack::client_state dsp_client::init() {
    jack::client_state state = jack::client::init();

    std::cout << "energia " << energy_window_size << std::endl;
    std::cout << "freq min " << period_minfreq << std::endl;
    std::cout << "freq max " << period_maxfreq << std::endl;
    std::cout << "min level " << period_minlevel << std::endl;
    std::cout << "window size " << period_window_size << std::endl;
    std::cout << "ring size " << period_ringsize << std::endl;

    jack_nframes_t sample_rate = jack::client::get_sample_rate();
    jack_nframes_t nframes = jack::client::get_buffer_size();

    if (state == jack::client_state::Running) {
        int size_buffer = energy_window_size * sample_rate / nframes;
        int capacity_ring_buffer = static_cast<int>(
            period_ringsize * sample_rate);
        int window_size = static_cast<int>(
            period_window_size * sample_rate);
        std::cout << "Buffer size Energy and Power: "
                  << size_buffer << std::endl;
        std::cout << "Capacity ring buffer Period: "
                  << capacity_ring_buffer << std::endl;
        std::cout << "Window size Period: " << window_size << std::endl;
        energy_queue.set_capacity(size_buffer);
        power_queue.set_capacity(size_buffer);
        ring_buffer.set_capacity(capacity_ring_buffer);
        correlation_signal.set_capacity(window_size);
    }
    return state;
}

void ::dsp_client::process_passthrough(jack_nframes_t nframes,
                                       const sample_t *const in,
                                       sample_t *const out) {
    memcpy(out, in, sizeof(sample_t) * nframes);
}

void dsp_client::process_volume_change(jack_nframes_t nframes,
                                       const sample_t *const in,
                                       sample_t *const out) {
    for (jack_nframes_t i = 0; i < nframes; i++) {
        out[i] = in[i] * volume;
    }
}

void dsp_client::calculate_energy_and_power(jack_nframes_t nframes,
                                            const sample_t *const signal) {
    // Check if energy mode is on
    if (!energy_mode)
        return;
    //  Caculate energy
    float energy = 0.0;
    for (jack_nframes_t i = 0; i < nframes; i++)
        energy += signal[i] * signal[i];

    // Add the actual energy to queue and update the accumalated energy
    energy_queue.push_back(energy);

    accumulated_energy += energy;
    power_queue.push_back(energy / nframes);
    accumulated_power += energy / nframes;

    // Get the sample rate
    jack_nframes_t sample_rate = jack::client::get_sample_rate();

    // If the queue is full, remove the first element
    while (energy_queue.size() + 1 > energy_window_size * sample_rate / nframes) {
        accumulated_energy -= energy_queue.front();
        energy_queue.pop_front();
        accumulated_power -= power_queue.front();
        power_queue.pop_front();
    }
}

bool dsp_client::process(jack_nframes_t nframes, const sample_t *const in,
                         sample_t *const out) {
    switch (current_mode) {
        case Mode::Passthrough:
            process_passthrough(nframes, in, out);
            break;
        case Mode::VolumeChange:
            process_volume_change(nframes, in, out);
            break;
        case Mode::Repeater:
            process_repeater(nframes, out);
            break;
        case Mode::Tuner:
            process_passthrough(nframes, in, out);
            break;
        case Mode::Autotune:
            process_autotune(nframes, out);
            break;
        default:
            break;
    }
    calculate_energy_and_power(nframes, in);
    get_data_period(nframes, in);
    return true;  // false if an error occurred
}

void dsp_client::change_mode(Mode new_mode) {
    current_mode = new_mode;
}

void dsp_client::adjust_volume(float delta) {
    volume += delta;
    if (volume > 10.0f)
        volume = 10.0f;
    if (volume < 0.0f)
        volume = 0.0f;
}

void dsp_client::get_data_period(jack_nframes_t nframes,
                                 const sample_t *const signal) {
    // Si no estamos en el modo de cálculo del período, salimos de la función
    if (!period_mode) {
        if (ring_buffer.size() > 0) {
            ring_buffer.clear();
        }
        return;
    }
    jack_nframes_t sample_rate = jack::client::get_sample_rate();
    // Cálculo de la energía de la señal actual
    float energy = 0.0;
    for (jack_nframes_t i = 0; i < nframes; i++)
        energy += signal[i] * signal[i];
    // Verificación del nivel mínimo de energía para comenzar la captura
    if (energy >= period_minlevel) {
        // Store the signal in bufer circular
        for (jack_nframes_t i = 0; i < nframes; ++i)
            ring_buffer.push_back(signal[i]);
        fail_counter_energy = 0;
    } else {
        fail_counter_energy += nframes;
    }
    if (fail_counter_energy / sample_rate > 0.1) {
        ring_buffer.clear();
    }
}

void dsp_client::set_energy_mode(bool mode) {
    energy_mode = mode;
    if (period_mode) {
        period_mode = false;
    }
}

void dsp_client::set_period_mode(bool mode) {
    period_mode = mode;
    if (energy_mode) {
        energy_mode = false;
    }
}

void dsp_client::calculate_period() {
    if (!period_mode) {
        period = -1;
        second_period = -1;
        correlation_signal.clear();
        ring_buffer.clear();
        counter_repeater = 0;
        ring_buffer_energy = 0;
        return;
    }
    // Get the size of the ring buffer
    int ring_buffer_size = ring_buffer.size();
    // Get the capacity of the correlation signal
    int windowsize = correlation_signal.capacity();

    // Get the i and n values for the autocorrelation
    int i = ring_buffer_size / 2;
    int n = i + windowsize;

    // Check if we have enough samples
    if (n > ring_buffer_size) {
        i = ring_buffer_size - windowsize;
        n = ring_buffer_size;
    }

    // If even that is not enough, exit
    if (i < 0) {
        period = -1;
        second_period = -1;
        correlation_signal.clear();
        counter_repeater = 0;
        ring_buffer_energy = 0;
        return;
    }

    // energy_buffer ring_buffer
    for (int k = 0; k < ring_buffer.size(); k++) {
        ring_buffer_energy += ring_buffer[k] * ring_buffer[k];
    }

    float sample_rate = jack::client::get_sample_rate();
    // Store the first and second peaks
    float first_peak_value = -1.0f;
    int first_peak_lag = -1;
    float second_peak_value = -1.0f;
    int second_peak_lag = -1;

    // Calculate the autocorrelation starting at 'i' and ending at 'n'
    // Init in lag=1 to avoid the peak in lag=0
    for (int lag = 1; lag <= n - i; ++lag) {
        float sum = 0.0f;
        for (int j = i; j < n - lag; ++j) {
            sum += ring_buffer[j] * ring_buffer[j + lag];
        }
        correlation_signal.push_back(sum);  // Push correlation signal

        // Calculate the frequency of the peak
        float freq = sample_rate / static_cast<float>(lag);

        // Check if frq is in range
        if (period_minfreq <= freq && freq <= period_maxfreq) {
            // Find the first peak
            if (first_peak_value < 0 || sum > first_peak_value) {
                second_peak_value = first_peak_value;
                second_peak_lag = first_peak_lag;
                first_peak_value = sum;
                first_peak_lag = lag;
            }
            // Find the second peak
            else if (second_peak_value < 0 || sum > second_peak_value) {
                second_peak_value = sum;
                second_peak_lag = lag;
            }
        }
    }

    //  Check if the two peaks are "more or less equal"
    if (first_peak_value >= 0 && second_peak_value >= 0) {
        float ratio = second_peak_value / first_peak_value;
        if (ratio >= 0.8 && ratio <= 1.2) {
            period = static_cast<float>(first_peak_lag) / sample_rate;
            second_period = static_cast<float>(second_peak_lag) / sample_rate;
        }
    }
}

void dsp_client::process_repeater(jack_nframes_t nframes,
                                  sample_t *const out) {
    if (!period_mode) {
        return;
    }

    float frequency = get_freq();
    jack_nframes_t sample_rate = jack::client::get_sample_rate();

    if (frequency <= 0) {
        for (jack_nframes_t i = 0; i < nframes; i++) {
            out[i] = 0;
        }
        counter_repeater = 0;
    } else {
        const float multiplier = 2 * M_PI * frequency / sample_rate;
        int n = nframes;
        n += counter_repeater;
        int i = counter_repeater;
        for (i; i < n; i++) {
            out[i] = volume * ring_buffer_energy * std::sin(i * multiplier);
        }
        counter_repeater = i;
    }
}

void dsp_client::process_tuner() {
    float frequency = get_freq();

    if (frequency <= 0) {
        freq_tuned = -1;
        note_tuned = "Sin sonido";
        frequency_difference = 0;
        return;
    }

    std::string closest_note;
    float closest_frequency;
    float min_frequency_difference = 200.0;
    float actual_frequency_difference;

    // diferencia 0 para indicar que esta afinado
    for (const auto &note_entry : notas) {
        const std::string &note_name = note_entry.first;
        float note_frequency = note_entry.second;
        float abs_frequency_difference = std::abs(note_frequency - frequency);

        if (abs_frequency_difference < min_frequency_difference) {
            min_frequency_difference = abs_frequency_difference;
            actual_frequency_difference = (note_frequency - frequency);
            closest_note = note_name;
            closest_frequency = note_frequency;
        }
    }

    freq_tuned = closest_frequency;
    note_tuned = closest_note;
    frequency_difference = actual_frequency_difference;
}

void dsp_client::process_autotune(jack_nframes_t nframes,
                                  sample_t *const out) {
    float frequency = get_freq_tuned();
    jack_nframes_t sample_rate = jack::client::get_sample_rate();

    if (frequency <= 0) {
        for (jack_nframes_t i = 0; i < nframes; i++) {
            out[i] = 0;
        }
        counter_repeater = 0;
    } else {
        const float multiplier = 2 * M_PI * frequency / sample_rate;
        int n = nframes;
        n += counter_repeater;
        int i = counter_repeater;
        for (i; i < n; i++) {
            out[i] = volume * ring_buffer_energy * std::sin(i * multiplier);
        }
        counter_repeater = i;
    }
}

void dsp_client::set_energy_window_size(float energy_window_size_) {
    energy_window_size = energy_window_size_;
}

void dsp_client::set_period_minfreq(int period_minfreq_) {
    period_minfreq = period_minfreq_;
}

void dsp_client::set_period_maxfreq(int period_maxfreq_) {
    period_maxfreq = period_maxfreq_;
}

void dsp_client::set_period_minlevel(float period_minlevel_) {
    period_minlevel = period_minlevel_;
}

void dsp_client::set_period_window_size(float period_window_size_) {
    period_window_size = period_window_size_;
}

void dsp_client::set_period_ringsize(float period_ringsize_) {
    period_ringsize = period_ringsize_;
}
