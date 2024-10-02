/**
 * waitkey.h
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

#ifndef _WAIT_KEY_H
#define _WAIT_KEY_H



/**
 * waitkey
 *
 * Waits for the given amount of milliseconds for a key to be pressed.
 *
 * If a timeout occurs (i.e. no key was pressed for the given amount
 * of time), then -1 is returned.
 *
 * This uses GNU/Linux low-level system calls.  The numbers returned
 * correspond to key codes, not ASCII codes.  They will be system
 * dependent, but the enum class key provides some helpful names.
 */
int waitkey(int timeout_ms=250);


#endif
