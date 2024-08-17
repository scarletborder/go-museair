package gomuseair

/*
#cgo CXXFLAGS: -std=c++17

#include "muse_air_capi.h"
*/
import "C"

type MuseAir interface {
	Hash(data []byte, seed uint64) (result uint64)
	Hash128(data []byte, seed uint64) (result []uint8)
	destructor()
}

type cgo_NormalMuseAir C.MuseAirNormal

type cgo_OptimizedMuseAir C.MuseAirOptimized

type NormalMuseAir struct {
	_isLittleEndian bool
	_cMuseAir       *cgo_NormalMuseAir
}

type OptimizedMuseAir struct {
	_isLittleEndian bool
	_cMuseAir       *cgo_OptimizedMuseAir
}
