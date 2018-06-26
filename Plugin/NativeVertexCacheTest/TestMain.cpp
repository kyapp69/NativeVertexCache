//! PrecompiledHeader Include.
#include "Plugin/PrecompiledHeader.h"

void RunTest_Types();
void RunTest_NVCAPI();
void RunTest_MemoryStream();
void RunTest_FileStream();


void main()
{
    RunTest_Types();
    RunTest_NVCAPI();
    RunTest_MemoryStream();
    RunTest_FileStream();
}
