#ifndef HELPER_H
#define HELPER_H

#define WM_HELPER_ADDINFO	(0x0400 + 300)
#define WM_UPDATE_INFO		(0x0400 + 301)

void AddInformation(const char* fmt, ...);
//void UpdateDialogInfo(const ServerState* _pState);
const char* GetRootPath();
void GetRootPath(char* _pszBuf, unsigned int _sz);

#endif