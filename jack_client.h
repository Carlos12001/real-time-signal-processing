/**
 * jack_client.h
 *
 * Copyright (C) 2023  Pablo Alvarado
 * EL5802 Procesamiento Digital de Señales
 * Escuela de Ingeniería Electrónica
 * Tecnológico de Costa Rica
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _JACK_CLIENT_H
#define _JACK_CLIENT_H

#include <jack/jack.h>
#include <ostream>

namespace jack {

  enum class client_state {
    Idle,
    Initializing,
    Running,
    ShuttingDown,
    Stopped,
    Error
  };
  
  /**
   * Jack client class
   *
   * This class wraps some basic jack functionality.
   *
   * It follows the monostate pattern to keep just one state
   * encompassing jack's functionality
   */
  class client {
  private:
    
    /// Part of monostate 
    static jack_client_t* _client_ptr;
    static client_state   _state;

    static jack_nframes_t _buffer_size;
    static jack_nframes_t _sample_rate;
    
  protected:
    
    static jack_port_t*   _input_port;
    static jack_port_t*   _output_port;
    
  public:
    typedef jack_default_audio_sample_t sample_t;
    
    /**
     * Creates a client in Idle state.  You still have to call init()
     * when ready to start processing.
     */
    client();
    client(const client&) = delete; // not copyable
    virtual ~client();

    /**
     * Process nframes from the input in array writing the output on
     * out.
     *
     * Return true if successful or false otherwise
     */
    virtual bool process(jack_nframes_t nframes,
                         const sample_t *const in,
                         sample_t *const out) = 0;
    
    virtual void shutdown();

    client& operator=(const client&) = delete; // not copyable

    /**
     * Initialize all necessary Jack callbacks, ports, etc. and
     * start processing.
     */
    client_state init();

    /**
     * Stop processing.  After calling this method, the application must
     * end, as no Jack client will be available anymore.
     */
    void stop();
    
    void set_sample_rate(const jack_nframes_t sample_rate);
    void set_buffer_size(const jack_nframes_t buffer_size);

    /**
     * Get input port
     */
    jack_port_t *const input_port() const;

    /**
     * Get output port
     */
    jack_port_t *const output_port() const;

    /**
     * Get sample rate
     */
    jack_nframes_t get_sample_rate();

    jack_nframes_t get_buffer_size();
    

  };
  
} // namespace jack

std::ostream& operator<<(std::ostream& os,const JackStatus& s);

#endif
