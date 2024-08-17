package gomuseair

func GetMuseAir(IsBlindFast bool) MuseAir {
	if IsBlindFast {
		return NewOptimizedMuseAir()
	} else {
		return NewNormalMuseAir()
	}
}

