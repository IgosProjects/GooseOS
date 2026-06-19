/*
 *	This file is part of gooseOS.
 *
 *	gooseOS is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	gooseOS is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with gooseOS.  If not, see <https://www.gnu.org/licenses/>.
 *
 *	Copyright(c) 2026 EyeDev
*/

#pragma once

namespace GooseOS {
    class Spinlock {
    private:
        // 0 = unlocked, 1 = locked
        volatile int lock_state = 0;

    public:
        void Lock() {
            // Attempt to swap with 1, if allready 1 fause
            while (__atomic_test_and_set(&lock_state, __ATOMIC_ACQUIRE)) {
                // Use pause to tell the CPU we are spinning and to safe power
                asm volatile("pause");
            }
        }

        void Unlock() {
            // Atomically clear the lock back to 0
            __atomic_clear(&lock_state, __ATOMIC_RELEASE);
        }
    };
}