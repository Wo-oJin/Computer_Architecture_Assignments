**PA0**: **Implement a simple string parser to get familiar with PASubmit system</br></br>**
         string과 관련된 library를 사용하지 않고, 사용자가 입력한 문자열에서 White Space를 기준으로 분리된 문자들을 저장하는 parse_command 함수 구현.</br></br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa0 에서 확인하실 수 있습니다.)</br></br></br>

**PA1**: **Translate MIPS assembly code into corresponding MIPS machine code</br></br>**
         사용자가 입력한 MIPS Assembly Language를 8자리 16진수로 표현된 MIPS Machine Code로 전환하는 translate() 함수 구현.</br></br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa1 에서 확인하실 수 있습니다.)</br></br></br>

**PA2**: **Implement a MIPS emulator that executes MIPS instructions loaded on the memory</br></br>**
         memory에 저장된 MIPS instruction을 실행하는 MIPS emulator을 구현.</br> emulator은 load_program, run_program, process_instruction 기능을 무조건 가짐.</br></br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa2 에서 확인하실 수 있습니다.)

    1. load_program()은 Input Stream에 주어지는 file에 있는 instrucion들을 memory에 저장합니다.
    2. run_program()은 memory에 있는 instruction을 하나하나 load 하여 실행합니다.
    3. process_instruction은 run_program()에서 불려지며 1개의 instruction을 처리하는 함수입니다.

</br></br>**PA3**: **Complete a cache simulator to better understand the memory hierarchy in computers</br></br>**
         CPU와 Main Memory 사이에 존재하는 Cache의 lw, sw 기능 구현</br></br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa3 에서 확인하실 수 있습니다.)

    1  Write-Back cache이며, Replacement Policy는 LRU 방식을 따릅니다.
    2. load_word()는 memory에 있는 data를 cache에 block 단위로 저장합니다.
    3. store_word()는 입력된 데이터를 cache에 저장합니다.
