/** @file base.cpp
 *
 * @brief This is a C++ version of the simple client, to demonstrate
 * the most basic features of JACK as they would be used by many
 * applications.
 */

#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "dsp_client.h"
#include "waitkey.h"
namespace po = boost::program_options;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl-C caught, cleaning up and exiting   " << std::endl;

        // Let RAII do the clean-up
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char* argv[]) {
    // ///////////////////////////////////////////

    po::options_description desc("Options");

    // Define las opciones de línea de comandos
    desc.add_options()("help,h", "Show help message")("energy,e", po::value<float>(), "Set energy window size")("minfreq", po::value<int>(), "Set minimum frequency")("maxfreq", po::value<int>(), "Set maximum frequency")("minlevel", po::value<float>(), "Set minimum level")("nwindow,n", po::value<float>(), "Set window size")("ringsize,r", po::value<float>(), "Set ring size");

    // Parsea los argumentos de línea de comandos
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Verifica si se especificó la opción de ayuda
    if (vm.count("help") || vm.count("h")) {
        std::cout << desc << std::endl;
        return 0;
    }

    bool flag_E_P = true;
    // std::cout << "Boost version: " << BOOST_LIB_VERSION << std::endl;

    std::signal(SIGINT, signal_handler);

    try {
        static dsp_client client;

        if (vm.count("energy") || vm.count("e")) {
            float energy_window_size = vm.count("energy") ? vm["energy"].as<float>() : vm["e"].as<float>();
            client.set_energy_window_size(energy_window_size);
        }

        if (vm.count("minfreq")) {
            int period_minfreq = vm["minfreq"].as<int>();
            client.set_period_minfreq(period_minfreq);
        }

        if (vm.count("maxfreq")) {
            int period_maxfreq = vm["maxfreq"].as<int>();
            client.set_period_maxfreq(period_maxfreq);
        }

        if (vm.count("minlevel")) {
            float period_minlevel = vm["minlevel"].as<float>();
            client.set_period_minlevel(period_minlevel);
        }

        if (vm.count("nwindow") || vm.count("n")) {
            float period_window_size = vm.count("nwindow") ? vm["nwindow"].as<float>() : vm["n"].as<float>();
            client.set_period_window_size(period_window_size);
        }

        if (vm.count("ringsize") || vm.count("r")) {
            float period_ringsize = vm.count("ringsize") ? vm["ringsize"].as<float>() : vm["r"].as<float>();
            client.set_period_ringsize(period_ringsize);
        }

        if (client.init() != jack::client_state::Running) {
            throw std::runtime_error("Could not initialize the JACK client");
        }

        // keep running until stopped by the user
        std::cout << "Press x key to exit" << std::endl;

        int key = -1;
        while (key != 'x') {
            key = waitkey(500);
            if (key > 0) {
                switch (key) {
                    case 'p':
                        client.change_mode(dsp_client::Mode::Passthrough);
                        std::cout << "Mode: Passthrough    " << std::endl;
                        break;
                    case 'v':
                        client.change_mode(dsp_client::Mode::VolumeChange);
                        std::cout << "Mode: Volume Change" << std::endl;
                        client.reset_volume();
                        break;
                    case '+':
                        if (client.get_current_mode() == dsp_client::Mode::VolumeChange ||
                            client.get_current_mode() == dsp_client::Mode::Repeater ||
                            client.get_current_mode() == dsp_client::Mode::Autotune) {
                            client.adjust_volume(0.05);  // Adjust this value as needed.
                            std::cout << std::fixed << std::setprecision(2)
                                      << "Volume: " << client.get_volume() << "\n"
                                      << std::endl;
                        }
                        break;
                    case '-':
                        if (client.get_current_mode() == dsp_client::Mode::VolumeChange ||
                            client.get_current_mode() == dsp_client::Mode::Repeater ||
                            client.get_current_mode() == dsp_client::Mode::Autotune) {
                            client.adjust_volume(-0.05);  // Adjust this value as needed.
                            std::cout << std::fixed << std::setprecision(2)
                                      << "Volume: " << client.get_volume() << "\n"
                                      << std::endl;
                        }
                        break;
                    case 'e':
                        std::cout << "\n"
                                  << std::endl;
                        client.set_energy_mode(!client.get_energy_mode());

                        std::cout << "Energy mode " << (client.get_energy_mode() ? "on" : "off") << "       " << std::endl;
                        break;

                    case 'E':
                        flag_E_P = !flag_E_P;
                        break;
                    case 'n':
                        client.set_period_mode(!client.get_period_mode());
                        std::cout << "Period mode " << (client.get_period_mode() ? "on" : "off") << "       " << std::endl;
                        break;
                    case 'r':
                        if (!client.get_period_mode()) {
                            client.set_period_mode(true);
                            client.reset_volume();
                        }
                        client.change_mode(dsp_client::Mode::Repeater);

                        std::cout << "Repeater mode on"
                                  << "       " << std::endl;
                        break;
                    case 't':
                        if (!client.get_period_mode()) {
                            client.set_period_mode(true);
                            client.reset_volume();
                        }
                        client.change_mode(dsp_client::Mode::Tuner);

                        std::cout << "Tuner mode on"
                                  << "       " << std::endl;

                        break;

                    case 'a':
                        if (!client.get_period_mode()) {
                            client.set_period_mode(true);
                            client.reset_volume();
                        }
                        client.change_mode(dsp_client::Mode::Autotune);

                        std::cout << "Autotune mode on"
                                  << "       " << std::endl;
                        break;
                    default:
                        if (key > 32) {
                            std::cout << "Key " << char(key) << " pressed " << std::endl;
                        } else {
                            std::cout << "Key " << key << " pressed " << std::endl;
                        }
                        break;
                }
            }
            client.calculate_period();
            client.process_tuner();
            if (client.get_energy_mode()) {
                if (flag_E_P == true) {
                    std::cout << std::fixed << std::setprecision(6)
                              << "Energy: " << client.get_energy()
                              << "\n"
                              << std::endl;
                } else {
                    std::cout << std::fixed << std::setprecision(6)
                              << "Power: " << client.get_power()
                              << "\n"
                              << std::endl;
                }
            } else if (client.get_period_mode()) {
                std::cout << std::fixed << std::setprecision(2)
                          << "Period: " << client.get_period()
                          << "\tFreq: " << client.get_freq()
                          << "\n"
                          << std::endl;

                if (client.get_current_mode() == dsp_client::Mode::Tuner) {
                    std::cout << "Frecuencia mas cercana: " << client.get_freq_tuned() << std::endl;
                    std::cout << "Corresponde a la nota: " << client.get_note_tuned() << std::endl;

                    if (std::abs(client.get_freq_diff()) < 0.5) {
                        std::cout << "Está afinado" << std::endl;
                    } else if (client.get_freq_diff() < 0) {
                        std::cout << "Debe bajar el tono" << std::endl;
                    } else {
                        std::cout << "Debe subir el tono" << std::endl;
                    }
                    /*
                    freq_tuned   = -1
                    si freq es -1 no hay nada sonado
                    si esta afinado 0.01+-
                    / frequcey_difference (RECUERDE EN EL IF SETEAR POR DEFECTO y en EL CONSTRCUTOR)

                  */
                } else if (client.get_current_mode() == dsp_client::Mode::Autotune) {
                    std::cout << "Periodo actual: " << 1 / client.get_freq() << std::endl;
                    std::cout << "Frecuencia actual: " << client.get_freq() << std::endl;
                    std::cout << "Frecuencia mas cercana: " << client.get_freq_tuned() << std::endl;
                    std::cout << "Corresponde a la nota: " << client.get_note_tuned() << std::endl;
                }
            }
        }

        client.stop();
    } catch (std::exception& exc) {
        std::cout << argv[0] << ": Error: " << exc.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
