package gomuseair

import (
	"encoding/binary"
	"runtime"
	"unsafe"
)

/*
#cgo CXXFLAGS: -std=c++17

#include "muse_air_capi.h"
*/
import "C"

func IsLittleEndian() bool {
	var buf [2]byte
	binary.LittleEndian.PutUint16(buf[:], 0x1)
	return buf[0] == 0x1
}

func NewNormalMuseAir() *NormalMuseAir {
	m := C.NewMuseAirNormal()
	ret := &NormalMuseAir{_isLittleEndian: IsLittleEndian(),
		_cMuseAir: (*cgo_NormalMuseAir)(m),
	}

	runtime.SetFinalizer(ret, (*NormalMuseAir).destructor)
	return ret
}

func NewOptimizedMuseAir() *OptimizedMuseAir {
	m := C.NewMuseAirOptimized()
	ret := &OptimizedMuseAir{_isLittleEndian: IsLittleEndian(),
		_cMuseAir: (*cgo_OptimizedMuseAir)(m),
	}

	runtime.SetFinalizer(ret, (*OptimizedMuseAir).destructor)
	return ret
}

func (m *NormalMuseAir) Hash(data []byte, seed uint64) (result uint64) {
	if m._isLittleEndian {
		C.MuseAirNormal_LittleHash((*C.MuseAirNormal)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result))
	} else {
		C.MuseAirNormal_BigHash((*C.MuseAirNormal)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result))
	}
	return
}

func (m *NormalMuseAir) Hash128(data []byte, seed uint64) (result []uint8) {
	result = make([]uint8, 16)
	if m._isLittleEndian {
		C.MuseAirNormal_LittleHash128((*C.MuseAirNormal)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result[0]))
	} else {
		C.MuseAirNormal_BigHash128((*C.MuseAirNormal)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result[0]))
	}
	return
}

func (m *OptimizedMuseAir) Hash(data []byte, seed uint64) (result uint64) {
	if m._isLittleEndian {
		C.MuseAirOptimized_LittleHash((*C.MuseAirOptimized)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result))
	} else {
		C.MuseAirOptimized_BigHash((*C.MuseAirOptimized)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result))
	}
	return
}

func (m *OptimizedMuseAir) Hash128(data []byte, seed uint64) (result []uint8) {
	result = make([]uint8, 16)
	if m._isLittleEndian {
		C.MuseAirOptimized_LittleHash128((*C.MuseAirOptimized)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result[0]))
	} else {
		C.MuseAirOptimized_BigHash128((*C.MuseAirOptimized)(m._cMuseAir), unsafe.Pointer(&data[0]), C.uint64_t(seed), unsafe.Pointer(&result[0]))
	}
	return
}

func (m *NormalMuseAir) destructor() {
	if m._cMuseAir != nil {
		C.DeleteMuseAirNormal((*C.MuseAirNormal)(m._cMuseAir))
	}
}

func (m *OptimizedMuseAir) destructor() {
	if m._cMuseAir != nil {
		C.DeleteMuseAirOptimized((*C.MuseAirOptimized)(m._cMuseAir))
	}
}
