package main

import (
    "fmt"
)

func squares() func() int {
    return func() int {
        var x int
        x++
        return x * x
    }
}

func main() {
    f := squares()
    fmt.Println(f())
    fmt.Println(f())

    g := squares()
    fmt.Println(g())
}

