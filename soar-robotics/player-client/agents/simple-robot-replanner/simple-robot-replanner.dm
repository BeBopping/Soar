71
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 simple-robot
ENUMERATION 8 1 initialize-simple-robot
SOAR_ID 9
ENUMERATION 10 1 wander
SOAR_ID 11
SOAR_ID 12
SOAR_ID 13
ENUMERATION 14 3 backward forward stop
FLOAT_RANGE 15 0.0 1.0
ENUMERATION 16 2 complete error
ENUMERATION 17 3 left right stop
FLOAT_RANGE 18 0.0 1.0
ENUMERATION 19 2 complete error
ENUMERATION 20 2 complete error
SOAR_ID 21
ENUMERATION 22 1 wait
ENUMERATION 23 2 operator state
ENUMERATION 24 2 multiple none
ENUMERATION 25 1 elaboration
ENUMERATION 26 2 no-change tie
SOAR_ID 27
STRING 28
SOAR_ID 29
SOAR_ID 30
STRING 31
FLOAT_RANGE 32 -Infinity Infinity
FLOAT_RANGE 33 0.0 1.0
SOAR_ID 34
INTEGER_RANGE 35 -2147483648 2147483647
SOAR_ID 36
SOAR_ID 37
FLOAT_RANGE 38 -Infinity Infinity
FLOAT_RANGE 39 -Infinity Infinity
SOAR_ID 40
ENUMERATION 41 1 wander-waypoint
SOAR_ID 42
ENUMERATION 43 1 choose-next-waypoint
SOAR_ID 44
SOAR_ID 45
FLOAT_RANGE 46 -Infinity Infinity
FLOAT_RANGE 47 -Infinity Infinity
SOAR_ID 48
FLOAT_RANGE 49 -Infinity Infinity
SOAR_ID 50
FLOAT_RANGE 51 -Infinity Infinity
FLOAT_RANGE 52 -Infinity Infinity
FLOAT_RANGE 53 0.0 1.0
SOAR_ID 54
ENUMERATION 55 1 go-to-waypoint
SOAR_ID 56
ENUMERATION 57 1 choose-destination
SOAR_ID 58
ENUMERATION 59 1 update-progress
SOAR_ID 60
ENUMERATION 61 1 update-map
SOAR_ID 62
ENUMERATION 63 1 give-up-on-move-to
SOAR_ID 64
ENUMERATION 65 1 state
ENUMERATION 66 1 give-up-on-path
ENUMERATION 67 1 return-to-source
SOAR_ID 68
ENUMERATION 69 1 go-to-waypoint
SOAR_ID 70
73
0 action-interval-time 32
0 attribute 23
0 choices 24
0 impasse 26
0 io 1
0 item 27
0 name 7
0 operator 4
0 operator 9
0 operator 21
0 operator 40
0 operator 42
0 operator 54
0 operator 56
0 operator 58
0 operator 60
0 operator 62
0 superstate 6
0 top-state 0
0 type 5
0 waypoints 44
1 input-link 2
1 output-link 3
2 self 36
2 time 34
3 move 11
3 move-to 50
3 rotate 12
3 stop 13
4 name 8
9 actions 29
9 name 10
11 direction 14
11 status 16
11 throttle 15
12 direction 17
12 status 19
12 throttle 18
13 status 20
21 name 22
21 random 25
27 name 28
29 dummy 30
30 dummy 31
30 throttle 33
34 seconds 35
36 current-location 37
37 x 38
37 y 39
40 name 41
42 name 43
44 at 45
44 waypoint 45
45 distance 49
45 next 48
45 x 46
45 y 47
50 throttle 53
50 x 51
50 y 52
54 name 55
56 name 57
58 name 59
60 name 61
62 name 63
64 name 66
64 operator 68
64 operator 70
64 superstate 0
64 top-state 0
64 type 65
68 name 67
70 name 69
