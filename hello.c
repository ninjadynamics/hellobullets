#include "neslib.h"
#include "stdint.h"

// link the pattern table into CHR ROM
//#link "chr_generic.s"

const unsigned char pal[16]={
  0x0f,0x00,0x10,0x30,
  0x0f,0x0c,0x21,0x32,
  0x0f,0x05,0x16,0x27,
  0x0f,0x0b,0x1a,0x29
};

void main(void) {
  
  int8_t vx, vy;
  
  uint8_t frame;
  uint8_t x, y;  
  uint8_t spr_id;
  uint8_t pp, i;
  
  int8_t bullet_vx[32];  
  uint8_t bullet_x[32];
  uint8_t bullet_y[32];
  uint8_t bullet_index;
  
  bool left;
  bool jump;
  bool shot;
  
  uint8_t tmp;
  
  pal_bg(pal);
  pal_spr(pal);

  #define FLOOR_Y ((23 * 8) - 1)
  
  x = 128 - 4;
  y = FLOOR_Y;

  for (x = 0; x < 32; ++x) {
    vram_adr(NTADR_A(x, 24));
    vram_put(0x8f);
  }

  // enable PPU rendering (turn on screen)
  ppu_on_all();
  
  shot = false;
  jump = false;
  bullet_index = 0;
  while (true) {
    
    // Read gamepad
    pp = pad_poll(0);       
      
    // Movement
         if (pp & PAD_LEFT ) { if (--vx < -2) vx = -2; left = true;  }
    else if (pp & PAD_RIGHT) { if (++vx > +2) vx = +2; left = false; }
    else vx = 0;
    
    // Jump
    if (pp & PAD_A) {
      if (y == FLOOR_Y && !jump) {
        jump = true;
        vy = -8;
      }
    } 
    else {
      jump = false;
    }
    
    // Shoot
    if (pp & PAD_B) {
      if (!shot) {
        shot = true;
        bullet_vx[bullet_index] = left ? -4 : 4;
        bullet_x[bullet_index] = x;
        bullet_y[bullet_index] = y;
        if (++bullet_index > 31) bullet_index = 0;
      }
    }
    else {
      shot = false;
    }    
    
    // Clear sprites
    oam_clear();
    
    // Draw player
    spr_id = oam_spr(x, y, 0x8f, 1, spr_id);
    
    // Update bullets
    for (i = 0; i < 32; ++i) {
      if (!bullet_vx[i]) continue;
      tmp = bullet_x[i]; bullet_x[i] += bullet_vx[i];
      if (
        (bullet_vx[i] > 0 && bullet_x[i] < tmp) ||
        (bullet_vx[i] < 0 && bullet_x[i] > tmp)
      ) {
        bullet_vx[i] = 0;
        continue;
      }
      // Draw bullets
      spr_id = oam_spr(bullet_x[i], bullet_y[i], 0x81, 2, spr_id);
    }
    
    // Apply gravity
    if (++frame % 2) vy += 1;
    
    // Update player position
    x += vx;
    y += vy;    
    if (y > FLOOR_Y) { 
      y = FLOOR_Y;
      vy = 0;
    }
    
    // Wait for vblank
    ppu_wait_nmi();
  };
}
