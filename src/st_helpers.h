#ifndef ST_HELPERS_H
#define ST_HELPERS_H

// Helper functions
void pushHistory(int y, int t, bool forceWrite = false);
bool isDark(int tl, int tr, int bl, int br);
bool isCloudy(int tl, int tr, int bl, int br);
void checkWatchdog();

#endif
