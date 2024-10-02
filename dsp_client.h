#ifndef _DSP_CLIENT_H
#define _DSP_CLIENT_H

#include <boost/circular_buffer.hpp>
#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_map>

#include "jack_client.h"

class dsp_client : public jack::client {
   public:
    // Modos de operación
    enum class Mode {
        Passthrough,
        VolumeChange,
        Repeater,
        Tuner,
        Autotune
    };

   private:
    Mode current_mode;
    float volume;  // Valor actual del volumen

    // For energy and power measure
    float energy_window_size;  // Window size in seconds
    bool energy_mode;

    // std::queue<float> energy_queue;
    boost::circular_buffer<float> energy_queue;

    float accumulated_energy;

    // std::queue<float> power_queue;
    boost::circular_buffer<float> power_queue;

    float accumulated_power;

    // For period calculation
    bool period_mode;
    float period_minfreq;
    float period_maxfreq;
    float period_minlevel;
    float period_window_size;
    float period_ringsize;
    float period;
    float second_period;
    bool capturing_frames;
    boost::circular_buffer<float> ring_buffer;
    boost::circular_buffer<float> correlation_signal;
    unsigned int fail_counter_energy;

    // repeater and autotune
    unsigned int counter_repeater;
    float ring_buffer_energy;
    float freq_tuned;
    std::string note_tuned;
    float frequency_difference;

    void process_passthrough(jack_nframes_t nframes,
                             const sample_t *const in,
                             sample_t *const out);

    void process_volume_change(jack_nframes_t nframes,
                               const sample_t *const in,
                               sample_t *const out);

    void process_repeater(jack_nframes_t nframes,
                          sample_t *const out);

    void process_autotune(jack_nframes_t nframes,
                          sample_t *const out);

    void calculate_energy_and_power(jack_nframes_t nframes,
                                    const sample_t *const signal);

    void get_data_period(jack_nframes_t nframes,
                         const sample_t *const signal);

   public:
    dsp_client();
    ~dsp_client();

    jack::client_state init();

    virtual bool process(jack_nframes_t nframes,
                         const sample_t *const in,
                         sample_t *const out) override;

    void change_mode(Mode new_mode);
    void adjust_volume(float delta);
    void reset_volume() { volume = 1.0f; }
    Mode get_current_mode() const { return current_mode; }
    float get_volume() const { return volume; }
    bool get_energy_mode() const { return energy_mode; }
    void set_energy_mode(bool mode);
    float get_energy() const { return accumulated_energy; }
    float get_power() const { return accumulated_power; }
    void set_period_mode(bool mode);
    bool get_period_mode() const { return period_mode; }
    float get_period() const { return period; }
    float get_second_period() const { return second_period; }
    float get_freq() const { return 1 / period; }
    void calculate_period();

    // std::string get_tuner();
    float get_freq_tuned() const { return freq_tuned; }
    std::string get_note_tuned() const { return note_tuned; }
    float get_freq_diff() const { return frequency_difference; }

    void set_energy_window_size(float energy_window_size_);
    void set_period_minfreq(int period_minfreq_);
    void set_period_maxfreq(int period_maxfreq_);
    void set_period_minlevel(float period_minlevel_);
    void set_period_window_size(float period_window_size_);
    void set_period_ringsize(float period_ringsize_);

    void process_tuner();
};

#endif
