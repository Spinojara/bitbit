#!/usr/bin/env python3

# bitbit, a bitboard based chess engine written in c.
# Copyright (C) 2022 Isak Ellmer
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import numpy
import ctypes
import torch
import pathlib

import model

class batch(ctypes.Structure):
    _fields_ = [
            ('actual_size', ctypes.c_size_t),
            ('ind_active', ctypes.c_int),
            ('ind1', ctypes.POINTER(ctypes.c_int32)),
            ('ind2', ctypes.POINTER(ctypes.c_int32)),
            ('eval', ctypes.POINTER(ctypes.c_float)),
    ]

    def get_tensors(self, device):
        eval = torch.from_numpy(numpy.ctypeslib.as_array(self.eval, shape = (self.actual_size, 1))).pin_memory().to(device = device, non_blocking = True)

        val1 = torch.ones(self.ind_active).pin_memory().to(device = device, non_blocking = True)
        val2 = torch.ones(self.ind_active).pin_memory().to(device = device, non_blocking = True)

        ind1 = torch.transpose(torch.from_numpy(numpy.ctypeslib.as_array(self.ind1, shape = (self.ind_active, 2))), 0, 1).pin_memory().to(device = device, non_blocking = True)
        ind2 = torch.transpose(torch.from_numpy(numpy.ctypeslib.as_array(self.ind2, shape = (self.ind_active, 2))), 0, 1).pin_memory().to(device = device, non_blocking = True)

        f1 = torch.sparse_coo_tensor(ind1, val1, (self.actual_size, model.FT_IN_DIMS + model.VIRTUAL), check_invariants = False).to(device = device, non_blocking = True)
        f2 = torch.sparse_coo_tensor(ind2, val2, (self.actual_size, model.FT_IN_DIMS + model.VIRTUAL), check_invariants = False).to(device = device, non_blocking = True)

        f1._coalesced_(True)
        f2._coalesced_(True)

        return f1, f2, eval

    def is_empty(self):
        return (self.actual_size == 0)


lib = ctypes.cdll.LoadLibrary(pathlib.Path(__file__).parent / "../../libbatch.so")

lib.batch_init.argtypes = None
lib.batch_init.restype = None

lib.next_batch.argtypes = [ctypes.c_void_p]
lib.next_batch.restype = ctypes.POINTER(batch)

lib.batch_open.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_double]
lib.batch_open.restype = ctypes.c_void_p

lib.batch_reset.argtypes = [ctypes.c_void_p]
lib.batch_reset.restype = None

lib.batch_close.argtypes = [ctypes.c_void_p]
lib.batch_close.restype = None