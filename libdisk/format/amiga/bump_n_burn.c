/*
 * disk/bump_n_burn.c
 * 
 * Custom format as used on Bump 'n' Burn by Grandslam.
 * 
 * Written in 2016 by Keir Fraser
 * 
 * RAW TRACK LAYOUT:
 *  u16 0x2291,0x2291
 *  u8  data[0x1810]      :: even block / odd block
 * No checksum. Check against pre-computed CRC16-CCITT values.
 * First data long contains header information (track number, disk number).
 * 
 * TRKTYP_bump_n_burn data layout:
 *  u8 sector_data[0x1810]
 */

#include <libdisk/util.h>
#include <private/disk.h>

static const uint16_t crcs[];

static void *bump_n_burn_write_raw(
    struct disk *d, unsigned int tracknr, struct stream *s)
{
    struct track_info *ti = &d->di->track[tracknr];

    while (stream_next_bit(s) != -1) {

        uint32_t dat[(ti->len * 2) / 4], header;
        uint32_t idx_off = s->index_offset_bc - 31;
        unsigned int disk_nr;
        void *block;

        if (s->word != 0x22912291)
            continue;

        stream_start_crc(s);
        if (stream_next_bytes(s, dat, 2*ti->len) == -1)
            goto fail;
        mfm_decode_bytes(bc_mfm_even_odd, ti->len, dat, dat);

        header = be32toh(dat[0]);
        if ((header >> 24) != tracknr)
            continue;
        if ((header & 0xfff0) != 0x3030)
            continue;

        disk_nr = (uint8_t)header - '0';
        if ((disk_nr > 6) || (tracknr > 159)
            || (s->crc16_ccitt != crcs[(disk_nr-1)*160+tracknr]))
            continue;

        /* IPF has normal-length tracks. Dump from Barry has long tracks. */
        stream_next_index(s);
        if (s->track_len_bc > 101500)
            ti->total_bits = 102800;

        ti->data_bitoff = idx_off;
        set_all_sectors_valid(ti);
        block = memalloc(ti->len);
        memcpy(block, dat, ti->len);
        return block;
    }

fail:
    return NULL;
}

static void bump_n_burn_read_raw(
    struct disk *d, unsigned int tracknr, struct tbuf *tbuf)
{
    struct track_info *ti = &d->di->track[tracknr];

    tbuf_bits(tbuf, SPEED_AVG, bc_raw, 32, 0x22912291);
    tbuf_bytes(tbuf, SPEED_AVG, bc_mfm_even_odd, ti->len, ti->dat);
}

static const uint16_t crcs[] = {
    /* Disk 1 */
    0x0000, 0xda90, 0x39ff, 0x79e7, 0x172c, 0x92ed, 0xb8ef, 0x8049, 
    0xc4a7, 0x91ac, 0x88d8, 0x22f3, 0xdf10, 0x5973, 0x5f9e, 0x13e7, 
    0x9b65, 0x94ea, 0x42ff, 0xbffc, 0xb1c4, 0xa2e8, 0xf2c9, 0xe2e2, 
    0x267f, 0x22e7, 0x3f37, 0x3842, 0x3230, 0xe6ab, 0xe6cc, 0x81a3, 
    0xa6b9, 0x8543, 0x8d9a, 0x250d, 0x1a11, 0x6795, 0xd3f3, 0x141d, 
    0x9d30, 0xeabc, 0x8031, 0xe7e8, 0x42d8, 0x099f, 0xe6a6, 0x2430, 
    0x8d4f, 0x9d96, 0x7ce7, 0x4d18, 0x783f, 0x1a34, 0x18e2, 0xfbdc, 
    0xcdc5, 0x294e, 0xd2d5, 0x8a07, 0x4556, 0x978a, 0x1181, 0x9ae9, 
    0xcbb1, 0xff0c, 0xae54, 0xb1b0, 0xc4e1, 0xfab8, 0x6b41, 0x9d72, 
    0xf4f9, 0x8aed, 0x87a9, 0x3546, 0x0dbc, 0xd5e1, 0xcc32, 0xdb07, 
    0x5187, 0xa5da, 0xe710, 0xf22d, 0x12d0, 0xa3d1, 0x5727, 0xff31, 
    0x0afc, 0xc2d4, 0x6c28, 0xf8d1, 0x182f, 0x967e, 0x6e82, 0x4783, 
    0x2374, 0x817e, 0xfebf, 0x623d, 0xf7f8, 0xfec4, 0x56b9, 0xb65b, 
    0x76e1, 0x2192, 0xbfb6, 0xe96c, 0x9c73, 0xb57e, 0x033e, 0xca47, 
    0x2e46, 0xd743, 0x3b76, 0x1e7f, 0x5b73, 0xf29a, 0x2413, 0xf914, 
    0xc376, 0x6a39, 0x1ab2, 0xb296, 0x4494, 0xc7c3, 0x6764, 0xf684, 
    0x04e9, 0x5d97, 0x8b67, 0xc0ba, 0x12a1, 0xf77c, 0xa0b3, 0x7100, 
    0xee8e, 0x117d, 0xb4d1, 0x36da, 0x511a, 0xa253, 0x3bd8, 0x5a37, 
    0xebb2, 0x6f47, 0xdac5, 0x805d, 0x6dc4, 0x823b, 0xeb25, 0x474b, 
    0xce85, 0xaaaf, 0x65fc, 0x9cb7, 0xd5a1, 0xd5f1, 0x2017, 0xbd9d, 
    /* Disk 2 */
    0xbcf3, 0x28d3, 0x1f84, 0xab54, 0x3f57, 0x9d2d, 0xd4ab, 0xd466, 
    0xd8b3, 0xf5fb, 0x8275, 0x8df0, 0x8fee, 0x07e4, 0xf044, 0x3217, 
    0x18d0, 0x1b69, 0xdf72, 0x658c, 0x07d5, 0x0d6b, 0x54f4, 0x16af, 
    0x21b4, 0x6b62, 0x54b2, 0x75be, 0xa83a, 0x31de, 0xf4a2, 0x464e, 
    0xe60f, 0xdeb6, 0xd39d, 0x33d1, 0x5ac4, 0x294f, 0x79f0, 0x865b, 
    0x8109, 0xc91f, 0x1290, 0xade9, 0x2d3a, 0x24ec, 0x7df5, 0x973b, 
    0x91ed, 0xb78b, 0x57ac, 0xa62b, 0x5066, 0x9bea, 0xf6f8, 0x594b, 
    0x1ff2, 0x56f7, 0xf684, 0x05a9, 0xf3da, 0x35be, 0x96d4, 0xced8, 
    0x0ba5, 0xce1f, 0xb093, 0x691f, 0xdcf5, 0x0b32, 0xbef5, 0x007b, 
    0xdaad, 0xc280, 0xab97, 0xc181, 0x16e4, 0x6637, 0xd653, 0x9f7f, 
    0xf2e8, 0x6673, 0x345e, 0xfcfc, 0x23cf, 0x09db, 0xc5fa, 0x17d2, 
    0xdd98, 0xe12c, 0xc9c5, 0xc7e3, 0xfd1a, 0x7dcb, 0x5cdc, 0x9d15, 
    0x7b74, 0x08b9, 0x1677, 0x8e73, 0x01dd, 0xdd64, 0x04ea, 0x80b9, 
    0xcb28, 0xab05, 0xc2b1, 0x3127, 0x9d15, 0x49bc, 0xf499, 0x2be8, 
    0xdc49, 0xdd46, 0xe35b, 0x5ad7, 0x78fb, 0xc439, 0xea4f, 0xd7b7, 
    0x2439, 0xe82d, 0x3315, 0x33da, 0x4d22, 0x6eff, 0x3213, 0xd9ae, 
    0xad5f, 0x52c5, 0x51a0, 0xe594, 0x4572, 0x25f8, 0xaba6, 0x92a9, 
    0x5908, 0x2c46, 0x9205, 0x9b17, 0x01a8, 0x5b7b, 0xfc6d, 0xec2a, 
    0x16c8, 0xbcbf, 0x0c59, 0x0bee, 0x925e, 0xcb82, 0xfa24, 0x7cd3, 
    0x3de9, 0xc23c, 0xd897, 0x756d, 0x3559, 0xb501, 0x4117, 0x0250, 
    /* Disk 3 */
    0x0adf, 0xb885, 0xffee, 0x11f4, 0x0d08, 0xf26c, 0xaf87, 0xc8a8, 
    0xe2bd, 0xdeef, 0xea8c, 0xf17f, 0x2672, 0xe0ee, 0xbd08, 0xdcaa, 
    0xdd84, 0xdacc, 0xf33b, 0x58ae, 0x0f21, 0xf826, 0x019c, 0xea10, 
    0xdef0, 0x76b5, 0x7ded, 0x0b8c, 0x393b, 0xdcbf, 0x1941, 0xdee7, 
    0xf9a9, 0xa9f3, 0x355a, 0x0bfb, 0x194f, 0xe0a1, 0x3ff1, 0x79c8, 
    0x2878, 0x8aad, 0xbac0, 0xc2a2, 0x3217, 0x8e9f, 0x9162, 0xeb9c, 
    0x7b8a, 0x2663, 0xa7c1, 0x6ee1, 0x316e, 0xfd91, 0xfea6, 0x9746, 
    0x6a80, 0x0e41, 0x8223, 0xaf30, 0x70de, 0x1293, 0x5488, 0x4c9e, 
    0x7f34, 0xf56b, 0x11f2, 0xe745, 0x4aef, 0x0ff2, 0xbadc, 0x8b84, 
    0x60b7, 0x9f9c, 0x51f1, 0xb07d, 0x592d, 0xea95, 0xc795, 0xdf53, 
    0x7d85, 0xd04d, 0xfc07, 0x9527, 0xa00d, 0x6ec4, 0x43ac, 0x1873, 
    0x62eb, 0x8e4a, 0xb5bb, 0x82d3, 0x895a, 0x7b53, 0x870f, 0x8814, 
    0x0abb, 0x9a4a, 0xaae6, 0x3d65, 0xce06, 0x44ca, 0x6016, 0x4f02, 
    0x4af6, 0xbe33, 0xd6de, 0x484e, 0x4dc4, 0x10a2, 0x9892, 0x053f, 
    0x9950, 0x6048, 0xbe5b, 0x2658, 0x11fd, 0x8885, 0xd48b, 0x15e6, 
    0x6ccb, 0xa1ce, 0xc3ba, 0x3a50, 0xd043, 0x3476, 0xf181, 0x0ef0, 
    0x5351, 0x68e8, 0x0556, 0xed98, 0xc138, 0xdeac, 0xc84a, 0x4296, 
    0x8b18, 0x2661, 0x98de, 0x2cad, 0xdc86, 0x8d28, 0xbf2e, 0xc8b3, 
    0xedd7, 0x16a8, 0x21f1, 0xa586, 0x681a, 0x7619, 0xbc0d, 0xc78b, 
    0xb951, 0x6c8d, 0x6852, 0xdbdc, 0xa83e, 0x1bb0, 0x1f6f, 0xace1, 
    /* Disk 4 */
    0xc3d9, 0x2f5e, 0x8be5, 0xb688, 0x6284, 0xcb4e, 0xaff4, 0xb891, 
    0xd2b8, 0x1cc3, 0x559d, 0x41fb, 0x87d6, 0xb81b, 0x7ada, 0xf7ca, 
    0xaa48, 0x4d54, 0xa718, 0x903c, 0xd39a, 0xc6f6, 0x436d, 0xb2b9, 
    0x8c4e, 0x3abb, 0x0a23, 0x3e44, 0x35d4, 0x94a3, 0xe194, 0x12bd, 
    0x583a, 0x0e36, 0xe2ec, 0xfa4d, 0xf85f, 0xaa0d, 0x7b9d, 0x52ef, 
    0x470c, 0xda69, 0xfc1f, 0x390a, 0x05d3, 0xfe89, 0xafe5, 0xb16e, 
    0x9214, 0x05aa, 0xe079, 0x7240, 0x605f, 0x2087, 0x9715, 0x52ca, 
    0x310d, 0x056f, 0x2b72, 0xde66, 0x3350, 0xeed4, 0x2122, 0x3529, 
    0x61e3, 0xe8eb, 0x99ad, 0x4737, 0xdd51, 0xc053, 0x1059, 0xd15c, 
    0x4fd7, 0xa32f, 0xa063, 0x387f, 0x3394, 0x9aa9, 0x69fc, 0x4cac, 
    0x7745, 0x916f, 0x14e0, 0xbf13, 0x842d, 0x5eb8, 0x0d74, 0x2320, 
    0x2e38, 0x7195, 0x8416, 0xb7b1, 0x155d, 0x4765, 0xd44c, 0xaaa9, 
    0x07f9, 0x576c, 0xd086, 0x0a2c, 0x40f4, 0x18a3, 0xe07f, 0xdccc, 
    0xedbc, 0xaf1f, 0x4363, 0x376d, 0xad06, 0x99b4, 0x7317, 0x948b, 
    0x1521, 0x6ead, 0xd048, 0xa5be, 0xebdf, 0x5644, 0xc798, 0x8679, 
    0x7977, 0x6941, 0xce26, 0xad8b, 0x0e4a, 0xc31a, 0xb91b, 0x775e, 
    0x3270, 0xfefe, 0x8521, 0x830e, 0x454d, 0xb229, 0xf21c, 0x0e20, 
    0x4cf3, 0xff7d, 0xfba2, 0x482c, 0x3bce, 0x8840, 0x8c9f, 0x3f11, 
    0xdc0a, 0x6f84, 0x6b5b, 0xd8d5, 0xab37, 0x18b9, 0x1c66, 0xafe8, 
    0xa289, 0x1107, 0x15d8, 0xa656, 0xd5b4, 0x663a, 0x62e5, 0xd16b, 
    /* Disk 5 */
    0x8e48, 0x724d, 0x9cdc, 0xb5c3, 0x740c, 0x2d07, 0xfbe7, 0xd548, 
    0x909f, 0x4fad, 0x82e3, 0xf1a9, 0x2a3b, 0xaafc, 0xc066, 0xc9c1, 
    0x828f, 0xe064, 0xd32d, 0x5d46, 0x6205, 0xb0c2, 0x52bd, 0x86da, 
    0xffd8, 0x307f, 0xb2f7, 0x7572, 0xbc3b, 0x31b4, 0x36ec, 0x4aa7, 
    0x3667, 0xff3d, 0x1361, 0x365a, 0xc0a2, 0xf9ea, 0x1202, 0xe9ae, 
    0x439d, 0xcf8f, 0xbcbb, 0xa00c, 0xf201, 0xbe83, 0x8014, 0xe941, 
    0xd5bc, 0x9ad7, 0xe3fb, 0x94fb, 0x867c, 0xb3ae, 0x487e, 0x004e, 
    0xa13f, 0x56af, 0x1483, 0x0d2f, 0x545c, 0x2240, 0xbd3f, 0x6a0c, 
    0x929e, 0x79b5, 0x1716, 0x237e, 0x0e09, 0x28f9, 0x9364, 0xfcbd, 
    0x39d4, 0x5aca, 0x8466, 0x0f57, 0x8529, 0x197a, 0xb5c1, 0x4234, 
    0xd11e, 0xf22b, 0x6155, 0x39e7, 0x5c6f, 0xb93e, 0x1a02, 0x4f61, 
    0x5611, 0x67d3, 0xb5e1, 0x66fe, 0xddca, 0xfb2d, 0xebe7, 0xae09, 
    0x7a83, 0x6c6c, 0xd5eb, 0xae24, 0x13d6, 0xbfe5, 0xbdb9, 0x1207, 
    0xcb02, 0xc9ec, 0x0ac5, 0xa392, 0x18ae, 0xbb24, 0xbdb2, 0x53c5, 
    0x5112, 0x7b55, 0x4bf4, 0x6abf, 0x3225, 0xd48d, 0xc209, 0x1a3d, 
    0x52ea, 0xc8b4, 0x6097, 0xd319, 0xa0fb, 0x1375, 0x17aa, 0xa424, 
    0x9cc1, 0x2f4f, 0x2b90, 0x981e, 0xebfc, 0x5872, 0x5cad, 0xef23, 
    0xe242, 0x51cc, 0x5513, 0xe69d, 0x957f, 0x26f1, 0x222e, 0x91a0, 
    0x72bb, 0xc135, 0xc5ea, 0x7664, 0x0586, 0xb608, 0xb2d7, 0x0159, 
    0x0c38, 0xbfb6, 0xbb69, 0x08e7, 0x7b05, 0xc88b, 0xcc54, 0x7fda, 
    /* Disk 6 */
    0xb051, 0x8384, 0x20df, 0x9ab3, 0xd594, 0x4db8, 0xac28, 0xd4df, 
    0x977d, 0x7683, 0xeb40, 0x2dff, 0xfba6, 0x2c38, 0x97a3, 0x013a, 
    0x74c5, 0x3e9f, 0xc48e, 0x2b92, 0x3f25, 0x39c2, 0x1a64, 0x3c7d, 
    0xf56d, 0x2e7c, 0x2ce7, 0x778a, 0xf601, 0x18a8, 0x1d67, 0x0c0a, 
    0x12ff, 0x3dba, 0x7e37, 0x80f2, 0x4c8a, 0x13e4, 0x1cbd, 0x4c98, 
    0x05a1, 0xed3c, 0x4f4a, 0x1900, 0xb0df, 0xff7a, 0x0c16, 0xf874, 
    0xf154, 0xf4da, 0xec53, 0x29a0, 0x54bb, 0xe992, 0x8635, 0x9902, 
    0xd12a, 0x34c4, 0xdf78, 0x88e8, 0x7e9d, 0x605e, 0xe5f3, 0x66ec, 
    0xf7fb, 0x6cc5, 0x0f28, 0x268d, 0xe090, 0x06c3, 0x569f, 0x9f9d, 
    0x4326, 0xe133, 0x15a7, 0x26b3, 0xa2a9, 0x98b0, 0x26ef, 0xbda6, 
    0x15dc, 0x8279, 0xcf14, 0xabd9, 0x9102, 0xa377, 0x6bcf, 0x3798, 
    0xde87, 0xd5c9, 0x27d0, 0x7827, 0x63c9, 0xc9bd, 0xf699, 0x2e4f, 
    0x426b, 0xa13d, 0xa758, 0x98e3, 0x473d, 0xd0de, 0x5589, 0x291c, 
    0x3cae, 0x54cb, 0x859d, 0xdeae, 0x3817, 0x03c6, 0x36da, 0x890a, 
    0xaa13, 0xb4f6, 0xb17c, 0x0b15, 0xac6e, 0xed91, 0x4673, 0x07a7, 
    0x6669, 0x1412, 0x0839, 0x1315, 0x0c6b, 0xc7f2, 0x2f6b, 0xe128, 
    0x1a57, 0x7887, 0x20d3, 0x8cc9, 0xb870, 0x23c3, 0xa2ce, 0x82ce, 
    0x2d0e, 0x6105, 0x3287, 0xd654, 0x8a47, 0x1638, 0x6ad9, 0xa169, 
    0xe565, 0xf1fc, 0xcda0, 0x46ad, 0xa66b, 0x86c1, 0x22af, 0x3190, 
    0x4937, 0x8f7f, 0x5b43, 0x382e, 0xeed3, 0xf842, 0xa17c, 0x4f13
};

struct track_handler bump_n_burn_handler = {
    .bytes_per_sector = 0x1810,
    .nr_sectors = 1,
    .write_raw = bump_n_burn_write_raw,
    .read_raw = bump_n_burn_read_raw
};

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
