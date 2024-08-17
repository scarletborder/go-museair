# go-museair
Golang version of https://github.com/eternal-io/museair

C++ file is from https://github.com/Twilight-Dream-Of-Magic/museair-cpp

## About this repo
Inspired from video [薄纱！高中生编写的哈希算法超越了先前的所有算法！(合作 K--Aethiax&Twilight-Dream) 王一大佬你在看嘛？ [2024-08-16]
](https://www.bilibili.com/video/BV1vTeuefEHN). Try to offer Golang version's interface for test.

## How to use
1. Get package
```bash
go get github.com/scarletborder/go-museair@v0.1.0
```

2. Example

```go
var seed uint64 = 0x123456789ABCDEF0
var data1 = "Hello, World!"
museAirNormal := gomuseair.NewNormalMuseAir()

res := museAirNormal.Hash([]byte(data1), seed)
fmt.Print(res)
```