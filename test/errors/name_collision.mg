display_size 5 5
world_window -5 5  -5 5

struct circle(r) {
    rectangle { -r -r  r r }
}

circle(r=4)
