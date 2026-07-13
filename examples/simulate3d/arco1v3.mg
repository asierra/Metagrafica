struct ARCO1() {
    bezier { 0 1  0.553427 0.998736  0.998736 0.553427  1 0 }
}
struct ARCO2() {
    fit(ARCO1, stretch=true) { 0 0.5  1 1 }
    fit(ARCO1, stretch=true) { 0 0.5  1 0 }
}
struct ARCO3() {
    fit(ARCO1, stretch=true) { 0.5 0.5  1 1 }
    fit(ARCO1, stretch=true) { 0.5 0.5  1 0 }
    fit(ARCO1, stretch=true) { 0.5 0.5  0 0 }
}
struct ARCO4() {
    fit(ARCO1, stretch=true) { 0.5 0.5  1 1 }
    fit(ARCO1, stretch=true) { 0.5 0.5  1 0 }
    fit(ARCO1, stretch=true) { 0.5 0.5  0 0 }
    fit(ARCO1, stretch=true) { 0.5 0.5  0 1 }
}
struct ARCO5() {
    fit(ARCO1, stretch=true) { 0.5 0  1 1 }
    fit(ARCO1, stretch=true) { 0.5 0  0 1 }
}
