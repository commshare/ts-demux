typedef enum
{
    DACF_NONE                                   = 0x00000000,
    /* No Container format used.  */
    DACF_ADTS                                   = 0x00000001,
    /* ADTS Container format for AAC.  */
    DACF_ADIF                                   = 0x00000002,
    /* ADIF Container format for AAC. */
    DACF_LATM                                   = 0x00000004
    /* LATM Container format for AAC*/
}       DFBAudioContainerFormat;
