void sg_colorlerp(uint8_t c0_r, uint8_t c0_g, uint8_t c0_b,
                  uint8_t c1_r, uint8_t c1_g, uint8_t c1_b,
                  float a,
                  uint8_t *out_r, uint8_t *out_g, uint8_t *out_b);

void sg_colorlerp_lin(uint8_t c0_r, uint8_t c0_g, uint8_t c0_b,
                      uint8_t c1_r, uint8_t c1_g, uint8_t c1_b,
                      float a,
                      uint8_t *out_r, uint8_t *out_g, uint8_t *out_b);
