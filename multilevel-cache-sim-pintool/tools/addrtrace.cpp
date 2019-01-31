/*********************** Author: Mainak Chaudhuri ****************/
  
#include <stdio.h>
#include "pin.H"
#include <inttypes.h>

#define LOG_BLOCK_SIZE 6
#define BLOCK_SIZE (1 << LOG_BLOCK_SIZE)
#define BLOCK_MASK 0x3f

FILE * trace;
PIN_LOCK pinLock;

// Print a memory read record
VOID RecordMemAccess(VOID * addr, UINT32 size, THREADID tid, int type)
{   
    UINT64 start_addr = (UINT64)addr;
    UINT64 end_addr =  start_addr + size - 1;
    UINT64 start_block = start_addr >> LOG_BLOCK_SIZE;
    UINT64 end_block = end_addr >> LOG_BLOCK_SIZE;

    PIN_GetLock(&pinLock, tid+1);
    for (UINT64 i=start_block; i<=end_block; i++) {
      // fprintf(trace, "%d %"PRIu32" %"PRIu64"\n", tid, size, i);
      fprintf(trace, "%d %"PRIu64" %d\n", tid, i, type);
    }
    // fprintf(trace, "--\n" );
    PIN_ReleaseLock(&pinLock);
}

VOID RecordInstructionAddress(VOID * addr, UINT32 size, THREADID tid)
{   
    UINT64 start_addr = (UINT64)addr;
    UINT64 end_addr =  start_addr + size - 1;
    UINT64 start_block = start_addr >> LOG_BLOCK_SIZE;
    UINT64 end_block = end_addr >> LOG_BLOCK_SIZE;

    PIN_GetLock(&pinLock, tid+1);
    for (UINT64 i=start_block; i<=end_block; i++) {
      // fprintf(trace, "%d %"PRIu32" %"PRIu64"\n", tid, size, i);
      fprintf(trace, "%d %"PRIu64" 2\n", tid, i);
    }
    // fprintf(trace, "--\n" );
    PIN_ReleaseLock(&pinLock);
}


// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.

    
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.


    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                IARG_MEMORYOP_EA, memOp, IARG_UINT32, INS_MemoryOperandSize(ins, memOp), IARG_THREAD_ID,IARG_UINT32,0,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemAccess,
                IARG_MEMORYOP_EA, memOp,  IARG_UINT32, INS_MemoryOperandSize(ins, memOp), IARG_THREAD_ID, IARG_UINT32,1,
                IARG_END);
        }
    }
    INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordInstructionAddress,
                IARG_INST_PTR, IARG_UINT32, INS_Size(ins), IARG_THREAD_ID, IARG_END);
}

VOID Fini(INT32 code, VOID *v)
{
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen("/data/abhikmr/pin-3.6/source/tools/multilevel-cache-sim-pintool/output/addrtrace.out", "wb");

    PIN_InitLock(&pinLock);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
