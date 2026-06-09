#pragma once

// Wait for any in-flight DMA blit, then push the framebuffer to the display.
void flushDisplay();
