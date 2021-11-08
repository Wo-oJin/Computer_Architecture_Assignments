### 컴퓨터 구조 과목시간에 수행한 과제들을 모아놓은 저장소입니다.
#### 각 과제별 수행 내용은 다음과 같습니다.

**PA0**: string과 관련된 library를 사용하지 않고, 사용자가 입력한 문자열에서 White Space를 기준으로 분리된 문자들을 저장하는 parse_command 함수 구현.</br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa0 에서 확인하실 수 있습니다.)
 
**PA1**: 사용자가 입력한 MIPS Assembly Language를 8자리 16진수로 표현된 MIPS Machine Code로 전환하는 translate() 함수 구현.</br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa1 에서 확인하실 수 있습니다.)

**PA2**: memory에 저장된 MIPS instruction을 실행하는 MIPS emulator을 구현. emulator은 load_program, run_program, process_instruction 기능을 무조건 가짐.</br>
         (정확한 과제 내용은 https://git.ajou.ac.kr/sslab/ca-pa2 에서 확인하실 수 있습니다.)

    1. load_program()은 Input Stream에 주어지는 file에 있는 instrucion들을 memory에 저장합니다.
    2. run_program()은 memory에 있는 instruction을 하나하나 load 하여 실행합니다.
    3. process_instruction은 run_program()에서 불려지며 1개의 instruction을 처리하는 함수입니다.
