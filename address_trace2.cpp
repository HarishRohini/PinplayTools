/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2015 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

#include "pin.H"
#include <iostream>
#include <fstream>
#include "pinplay.H"
PINPLAY_ENGINE pinplay_engine;
KNOB<BOOL> KnobPinPlayLogger(KNOB_MODE_WRITEONCE,
                      "pintool", "log", "0",
                      "Activate the pinplay logger");
KNOB<BOOL> KnobPinPlayReplayer(KNOB_MODE_WRITEONCE, "pintool", "replay", "0",
        "Activate the pinplay replayer");
/* ===================================================================== */
/* Names of malloc and free */
/* ===================================================================== */
#if defined(TARGET_MAC)
#define MALLOC "_malloc"
#define FREE "_free"
#else
#define MALLOC "malloc"
#define FREE "free"
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile, TraceFile2;
ofstream OutFile;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "malloctrace.out", "specify trace file name");
KNOB<string> KnobOutputFile2(KNOB_MODE_WRITEONCE, "pintool",
    "o2", "assembly_trace.out", "specify trace file name");

/* ===================================================================== */


/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */

static UINT64 icount = 0;
int malloc_flag = 1;
int realloc_flag = 1;
int calloc_flag = 1;
int free_flag = 1;
string malloc_rw="";
stringstream strm, strm2, strm3, strm4, strm5;
ADDRINT free_size;
VOID* free_end_address=(void *)0x7f5f8eac62bf;//0x7f5f8eac62bf
VOID* free_start_address=(void *)0x7f5f8e702000;//0x7f5f8e702000
UINT64 region_start_count = 12270006257;
UINT64 region_end_count = 12300006261;
//ADDRINT free_end_address=0x7f5f8eac62bf;
//ADDRINT free_start_address=0x7f5f8e702000;
string free_ip;
string free_name, alloc_name;
map<long, string> inst_map;
bool in_range=false;
char str2[10];
// This function is called before every instruction is executed
VOID docount() 
{ 
    icount++;
    if((region_start_count <= icount) && (icount <= region_end_count))
	in_range = true;
    else if(icount > region_end_count)
    {
	TraceFile.close();
        //OutFile.close();
        for (std::map<long, string>::iterator it=inst_map.begin(); it!=inst_map.end(); ++it)
	    TraceFile2 << (void *)it->first <<" : "<< it->second.c_str()<<endl;
        TraceFile2.close();
        exit(0);
    }
    else {
    }
}

VOID RecordMemRead(VOID * ip, VOID * addr)
{
    //fprintf(trace,"%p: R %p\n", ip, addr);
    if(in_range){
    if(!free_flag)
    {
	if((free_start_address <= ip) && (ip <= free_end_address))
        {
	}
	else{
		TraceFile<<malloc_rw;
		TraceFile<<free_name<<endl;
        	free_flag = 1;
		malloc_rw = "";
		free_name = "";
	}
    }
    if (malloc_flag && realloc_flag && calloc_flag && free_flag)
    TraceFile<< ip <<" R "<<addr<<endl;
    else {
	strm << ip;
	strm2 << addr;
        free_ip = strm.str();
	malloc_rw += strm.str() + " R "+ strm2.str()+"\n"; 
	strm.str("");
	strm2.str("");
    }
    }
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
    //fprintf(trace,"%p: W %p\n", ip, addr);
    if(in_range) {
    if(!free_flag)
    {
	if((free_start_address <= ip) && (ip <= free_end_address))
        {
	}
	else{
		TraceFile<<malloc_rw;
		TraceFile<<free_name<<endl;
        	free_flag = 1;
		malloc_rw = "";
		free_name = "";
	}
    }

    if (malloc_flag && realloc_flag && calloc_flag && free_flag)
    TraceFile<< ip <<" W "<<addr<<endl;
    else {
        strm << ip;
        strm2 << addr;
	malloc_rw += strm.str() + " W " + strm2.str()+"\n";
        strm.str("");
	strm2.str("");
    }
    }
}

VOID Arg1Before(CHAR * name, ADDRINT size)
{
    if(strcmp(name,"malloc")==0){
	malloc_flag = 0;
        //TraceFile << name << "(" << size << ")" << endl;
        strm4 << (void *)size;
        alloc_name = "malloc("+strm4.str()+") : ";
        strm4.str("");

    }

//cout<<name<<" : "<<size<<endl;
    if(strcmp(name,"free")==0 && in_range){
    	free_size = size;
	free_flag = 0;
	strm3 << (void *)size;
        free_name = "free("+strm3.str()+")";
        //cout<<"free("<<size<<")"<<endl;
	strm3.str("");
    }
    else if(strcmp(name,"free")==0 && !in_range) {
	TraceFile<<"free("<<size<<")"<<endl;
    }
    else{
    }
}

VOID MallocAfter(ADDRINT ret)
{
    TraceFile << alloc_name << ret << endl;
    if(in_range)
    TraceFile<< malloc_rw;
    malloc_flag = 1;
    malloc_rw = "";
    alloc_name = "";

}

VOID ReallocBefore(ADDRINT arg1, ADDRINT arg2)
{
    realloc_flag = 0;
    //TraceFile<<"realloc("<<arg1<<","<<arg2<<")\n";
    strm4 << (void *)arg1;
    strm5 << (void *)arg2;
    alloc_name = "realloc("+strm4.str()+","+strm5.str()+") : ";
    strm4.str("");
    strm5.str("");
}

VOID ReallocAfter(ADDRINT ret)
{
	
   TraceFile <<alloc_name<< ret<<"\n";
   if(in_range)
   TraceFile<<malloc_rw;
   realloc_flag = 1;
   malloc_rw = "";
   alloc_name="";
   
}

VOID CallocBefore(ADDRINT arg1, ADDRINT arg2)
{
    calloc_flag = 0;
    //TraceFile<<"calloc("<<arg1<<","<<arg2<<")\n";
    strm4 << (void *)arg1;
    strm5 << (void *)arg2;
    alloc_name = "calloc("+strm4.str()+","+strm5.str()+") : ";
    strm4.str("");
    strm5.str("");

}

VOID CallocAfter(ADDRINT ret)
{
	
   TraceFile << alloc_name<< ret<<"\n";
   if(in_range)
   TraceFile<<malloc_rw;
   calloc_flag = 1;
   malloc_rw = "";
   alloc_name="";
}

VOID Routine(RTN rtn, VOID *v)
{
//TraceFile<< "malloc2 "<<endl;
if (RTN_Name(rtn) == "__libc_malloc")
    {
    //TraceFile<< "malloc "<<endl;
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);

        // Instrument malloc() to print the input argument value and the return value.
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
                       IARG_ADDRINT, MALLOC,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

        RTN_Close(rtn);
    }
    }
    //RTN freeRtn = INS_Rtn(ins);
    if (RTN_Name(rtn) == "__libc_free")
    {
    // Find the free() function.
    //RTN freeRtn = RTN_FindByName(img, FREE);
    if (RTN_Valid(rtn))
    {

        RTN_Open(rtn);
        // Instrument free() to print the input argument value.
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
                       IARG_ADDRINT, FREE,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
	//RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)free_end,
          //              IARG_END);
	
        RTN_Close(rtn);
	
    }
    //TraceFile<<"free("<<free_size<<")"<<endl;
    }

    if (RTN_Name(rtn) == "__libc_realloc")
    {
	if (RTN_Valid(rtn))
    	{
		RTN_Open(rtn);
		// Instrument free() to print the input argument value.
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)ReallocBefore,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			       IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
			       IARG_END);
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)ReallocAfter,
                               IARG_FUNCRET_EXITPOINT_VALUE,
                               IARG_END);
	
		RTN_Close(rtn);
	
    	}
    }

    if (RTN_Name(rtn) == "__libc_calloc")
    {
	if (RTN_Valid(rtn))
    	{
		RTN_Open(rtn);
		// Instrument free() to print the input argument value.
		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)CallocBefore,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			       IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
			       IARG_END);
		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)CallocAfter,
                               IARG_FUNCRET_EXITPOINT_VALUE,
                               IARG_END);
	
		RTN_Close(rtn);
	
    	}
    }
}

    
// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{

    // Insert a call to docount before every instruction, no arguments are passed
    //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
/*     RTN mallocRtn = INS_Rtn(ins);
   
    if (RTN_Name(mallocRtn) == "__libc_malloc")
    {
    //TraceFile<<<<endl;
    TraceFile<<INS_Address(ins)<<" : "<<INS_Disassemble(ins)<<" : "<<RTN_Name(mallocRtn)<<endl;
    }
    //RTN freeRtn = INS_Rtn(ins);
    if (RTN_Name(mallocRtn) == "__libc_free")
    {
    // Find the free() function.
    //RTN freeRtn = RTN_FindByName(img, FREE);
    //TraceFile<<RTN_Name(freeRtn)<<endl;
    TraceFile<<INS_Address(ins)<<" : "<<INS_Disassemble(ins)<<" : "<<RTN_Name(mallocRtn)<<endl;
    }
*/
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);

    string inst = INS_Disassemble(ins);

    if (inst.at(0)=='r'&&inst.at(1)=='e'&&inst.at(2)=='p'&&inst.at(3)==' ') {
	if (inst_map.find(INS_Address(ins)) == inst_map.end()) {
		sprintf( str2, "%u", INS_MemoryOperandCount(ins));
	    	inst_map[INS_Address(ins)] = inst + " : " + str2 ;
	}
	else {
	}
    }

    UINT32 memOperands = INS_MemoryOperandCount(ins);

    //cout<<INS_Address(ins)<<" : "<<INS_Disassemble(ins)<<"\n";

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
	    if (inst_map.find(INS_Address(ins)) == inst_map.end()) {
		sprintf( str2, "%u", memOperands);
	    	inst_map[INS_Address(ins)] = INS_Disassemble(ins) + " : " + str2;
	    }
	    else {
	    }

            //TraceFile<<INS_Address(ins)<<" : "<<RTN_Name(INS_Rtn(ins))<<endl;
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
	    if (inst_map.find(INS_Address(ins)) == inst_map.end()) {
		sprintf( str2, "%u", memOperands);
	    	inst_map[INS_Address(ins)] = INS_Disassemble(ins) + " : " + str2;
	    }
	    else {
	    }
	    //TraceFile<<INS_Address(ins)<<" : "<<RTN_Name(INS_Rtn(ins))<<endl;
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
    }

}
 

VOID Image(IMG img, VOID *v) 
{
TraceFile << "Loading " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl;
}
/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */
   
/* ===================================================================== */
VOID Fini(INT32 code, VOID *v)
{
    TraceFile.close();
    //OutFile.close();
    for (std::map<long, string>::iterator it=inst_map.begin(); it!=inst_map.end(); ++it)
	TraceFile2 << (void *)it->first <<" : "<< it->second.c_str()<<endl;
    TraceFile2.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    cerr << "This tool produces a trace of calls to malloc." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    // Initialize pin & symbol manager
    PIN_InitSymbols();
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    pinplay_engine.Activate(argc, argv,
      KnobPinPlayLogger, KnobPinPlayReplayer);
    
    // Write to a file since cout and cerr maybe closed by the application
    //OutFile.open(KnobOutputFile.Value().c_str());
    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile2.open(KnobOutputFile2.Value().c_str());
    TraceFile << hex;
    TraceFile2 << hex;
    TraceFile.setf(ios::showbase);
    TraceFile2.setf(ios::showbase);
    cout << hex;
    cout.setf(ios::showbase);
    // Register Image to be called to instrument functions.
    //IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    RTN_AddInstrumentFunction(Routine, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
