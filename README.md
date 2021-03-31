# cs19b005_cs19b006_pipelined_spim

This is pipelined simulator cs19b0005_cs19b006_spim made by team Run&Execute

Status : Working
Conditions : 	Due to time unavialabilty the code has been made upto written bubble sort code(not hard coded) however with few modifications this simulator can be applied for any MIPS code and those
		modications ideas  are briefly are discussed below.

		As the code from phase1 needed to go through some changes some subprogram lines (mainly function) had become unnecessary and they haven't been removed as they are not been called there is no 
		delay due to those unnecessay subprograms

		The made changes are previously it's an unpipelined. so we made them into 5 partitioned stages and get result from every for stage to perform next stage

APPROACH:

Our approach is purely dependent on how represent the pipelining diagram of a code in a paper

For this we constructed a 2 dimensional dynamic array
with every unit show status of stage and to what instruction is pipeling performed

This is done by taking the basic unit as struct 
This struct represents what stages has been made is any stage on that cycle has benn operated at that cycle and also shows if there is a stall
Now the 2 dimensional dynamic array is made for doing pipelining with and without data forwarding

The challenge here is how to make the pipelining as we do on paper:

The major task here is to identify the stalls, where we get them and for which instructions we get them

We get stalls basically if there are two  or three dependent instructions causing hazard
These are possibilities for getting stalls:
Basically each stage is handled by each latch so the current instruction when undergo the same stage at same cycle or before completion of same stage for prev instruction
If the registers at ID/RF are depending on prev 2 destination registers(prev 3rd dest reg won't be depending because by time of RF write back would have benn completed with difference of half cycle)
		the ID/RF for 1st time and in some cases there will be  stall and register fetched again

This kind of data forwarding can't be applied if we don't get the result to be write bacck by EX stage(examples: instructions with opcode la,lw)

So we compare every destination of prev 2 instructions' register with our source registers
This need not been done if the previous opcodes have only source register (instructions with opcode j,beq,bne,sw,...)

In the code the 1st operand has been taken mostly as destination register so we needed to add extra conditions where 1st operand after the opcode isn't dest register

Using this principle we can make any data forwarding pipeline for any kind of instruction


This is the case of data forwarding but for the case of not forwarding the Register Fetch only happens at the first time and fetcheing of registers happens after write back of depending instruction
in case of deopendecy hazards

With this principle we can make data unforwarded pipelining
