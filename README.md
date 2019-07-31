# self-vm
VM that can self-interpret itself
Copyright (C) 2019 Tomasz Dobrowolski

# license

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

# usage

Run "vm recursive_interpreter.code" to calculate 6th Fibonacci number
with 5 layers of VM self-interpretation, which is a C++ VM that is interpreting
a VM interpreter that is interpreting a VM interpreter that is interpreting a VM
interpreter that is interpreting a VM interpreter that is interpreting a Fibonacci
test program.

At the last layer, every cycle of Fibonacci test program corresponds to 33045174
cycles of the first layer. So you can expect any test program to be very very
slow at this point ;P Which makes the whole project pointless, which is exactly
the point of it!

At the end it will print 13, which is indeed a 6th Fibonacci number,
as in [ 1, 1, 3, 5, 8, 13 ] sequence.
And VM memory dump (near 0 address).
