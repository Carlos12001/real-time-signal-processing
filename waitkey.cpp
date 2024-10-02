/**
 * waitkey.cpp
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

#include "waitkey.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <iostream>

int waitkey(int timeout_ms) {

  class raii {
  private:
    termios _original_termios;
    termios _new_termios;
    int _flags=0;
  public:
    raii() {
       // Save the original terminal attributes
      tcgetattr(STDIN_FILENO,&_original_termios);

      // Set the terminal to non-canonical mode
      _new_termios = _original_termios;
      _new_termios.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &_new_termios);

      // Set standard input to non-blocking mode
      _flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, _flags | O_NONBLOCK);

      //std::cout << "Reconfiguring terminal" << std::endl;
    }
    
    ~raii() {
      // Restore the original terminal attributes and file descriptor flags
      tcsetattr(STDIN_FILENO, TCSANOW, &_original_termios);
      fcntl(STDIN_FILENO, F_SETFL, _flags);

      std::cout << "Restoring terminal" << std::endl;

    }
  };

  // Set terminal and restores at the end of program
  static raii init_terminal;

  // Wait for input using select with a timeout
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(STDIN_FILENO, &read_fds);
  struct timeval timeout;
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;
  int ret = select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout);
  
  int c = -1;
  if (ret > 0) {
    char ch;
    if (read(STDIN_FILENO, &ch, 1) == 1) {
      c = ch;
    }
  }
  
  return c;
}
