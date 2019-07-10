#ifndef PACKET_H
#define PACKET_H
typedef union {
    uint16_t packet_data;

    // struct
    // {
    //     uint8_t first_byte_data;
    //     uint8_t second_byte_data;
    // };
    struct
    {
        uint8_t player_id : 8;

        uint8_t kick : 1;
        uint8_t up : 1;
        uint8_t down : 1;
        uint8_t left : 1;
        uint8_t right : 1;
        /* remaining 3 bits unused... */
    };
} packet;

#endif