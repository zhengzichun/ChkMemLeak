#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

#include <checkleak/checkleak.h>
#include <backtrace/Backtrace.h>
#include <time.h>

extern "C" void get_malloc_leak_info(
    uint8_t **info, size_t *overallSize, size_t *infoSize,
    size_t *totalMemory, size_t *backtraceSize);

extern "C" void free_malloc_leak_info(uint8_t *info);

void checkLeak()
{
    std::unique_ptr<Backtrace> backtrace(Backtrace::Create(
		BACKTRACE_CURRENT_PROCESS, BACKTRACE_CURRENT_THREAD, NULL));

    backtrace->Unwind(0);

    ostringstream fileName;
    fileName << "/data/checkleak_" << backtrace->Pid() << "_" << time((time_t *)NULL) << ".csv";

    ofstream outFile(fileName.str());
    if (outFile)
		outFile << "size,duplications,backtrace" << endl;
    else
		return;

	uint8_t *info = NULL;
    size_t overallSize = 0, infoSize = 0, totalMemory = 0, backtraceSize = 0;
    get_malloc_leak_info(
		&info, &overallSize, &infoSize, &totalMemory, &backtraceSize);

    if (!info)
		return;

    size_t recordCount = overallSize / infoSize;
    size_t index = 0;
    uint8_t *ptr = info;

    while (index < recordCount)
    {
		size_t allocMemSize = *reinterpret_cast<size_t *>(ptr);
		ptr += sizeof(size_t);
		size_t allocTimes = *reinterpret_cast<size_t *>(ptr);
		ptr += sizeof(size_t);

		outFile << allocMemSize << "," << allocTimes << ",\"";

		for (size_t i = 0; i < backtraceSize; ++i)
		{
			uintptr_t pc = *reinterpret_cast<uintptr_t *>(ptr + i * sizeof(uintptr_t));
			if (!pc)
				break;

			backtrace_map_t map;
			backtrace->FillInMap(pc, &map);
			uintptr_t offset = 0;
			string funcName = backtrace->GetFunctionName(pc, &offset);

			uintptr_t relative_pc = BacktraceMap::GetRelativePc(map, pc);
			outFile << "#" << setw(2) << setfill('0') << i << " pc 0x" << hex << relative_pc << dec << " " << map.name;
			if (!funcName.empty())
			{
				outFile << " (" << funcName << "+0x" << hex << offset << dec << ")";
			}
			outFile << endl;
		}

		outFile << "\"" << endl;
		ptr += sizeof(uintptr_t) * backtraceSize;
		++index;
    }
    free_malloc_leak_info(info);
    outFile.close();
}