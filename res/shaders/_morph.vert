
uniform float u_morphWeights[MORPH_TARGET_COUNT];

#if MORPH_TARGET_COUNT > 0
    in vec3 a_morphTarget0;
#endif
#if MORPH_TARGET_COUNT > 1
    in vec3 a_morphTarget1;
#endif
#if MORPH_TARGET_COUNT > 2
    in vec3 a_morphTarget2;
#endif
#if MORPH_TARGET_COUNT > 3
    in vec3 a_morphTarget3;
#endif
#if MORPH_TARGET_COUNT > 4
    in vec3 a_morphTarget4;
#endif
#if MORPH_TARGET_COUNT > 5
    in vec3 a_morphTarget5;
#endif
#if MORPH_TARGET_COUNT > 6
    in vec3 a_morphTarget6;
#endif
#if MORPH_TARGET_COUNT > 7
    in vec3 a_morphTarget7;
#endif


vec3 getMorphPosition(vec3 pos) {
    #if MORPH_TARGET_COUNT > 0
        pos += (u_morphWeights[0] * a_morphTarget0);
    #endif
    #if MORPH_TARGET_COUNT > 1
        pos += (u_morphWeights[1] * a_morphTarget1);
    #endif
    #if MORPH_TARGET_COUNT > 2
        pos += (u_morphWeights[2] * a_morphTarget2);
    #endif
    #if MORPH_TARGET_COUNT > 3
        pos += (u_morphWeights[3] * a_morphTarget3);
    #endif
    #if MORPH_TARGET_COUNT > 4
        pos += (u_morphWeights[4] * a_morphTarget4);
    #endif
    #if MORPH_TARGET_COUNT > 5
        pos += (u_morphWeights[5] * a_morphTarget5);
    #endif
    #if MORPH_TARGET_COUNT > 6
        pos += (u_morphWeights[6] * a_morphTarget6);
    #endif
    #if MORPH_TARGET_COUNT > 7
        pos += (u_morphWeights[7] * a_morphTarget7);
    #endif

    return pos;
}
