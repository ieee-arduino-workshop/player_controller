/**
 * @brief: data struct 16 bits
 * 
 *  
 * 
 * */

typedef union {
    uint16_t packet_data;

    struct
    {
        uint8_t player_id : 8; //player ID
        uint8_t kick : 1;   //kick button

        // GYRO movement data
        uint8_t up : 1;     
        uint8_t down : 1;
        uint8_t left : 1;
        uint8_t right : 1;
        /* remaining 3 bits unused... */
    };
} packet;