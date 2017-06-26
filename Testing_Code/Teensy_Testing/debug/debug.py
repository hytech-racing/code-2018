import curses
import serial
import sys

def main():
    if len(sys.argv) != 2:
        print('Usage:')
        print('debug.py <serial device>')
        quit()
    global screen
    screen = curses.initscr()
    screen.border(0)
    screen.addstr(0,5,'HYTECH RACING 2016 VEHICLE SERIAL DEBUGGER')
    screen.addstr(3,5,'RMS INVERTER')
    screen.addstr(4,5,'MODULE A TEMP: ')
    screen.addstr(5,5,'MODULE B TEMP: ')
    screen.addstr(6,5,'MODULE C TEMP: ')
    screen.addstr(7,5,'GATE DRIVER BOARD TEMP: ')
    screen.addstr(8,5,'RTD 4 TEMP: ')
    screen.addstr(9,5,'RTD 5 TEMP: ')
    screen.addstr(10,5,'MOTOR TEMP: ')
    screen.addstr(11,5,'TORQUE SHUDDER: ')
    screen.addstr(12,5,'MOTOR ANGLE: ')
    screen.addstr(13,5,'MOTOR SPEED: ')
    screen.addstr(14,5,'ELEC OUTPUT FREQ: ')
    screen.addstr(15,5,'DELTA RESOLVER FILT: ')
    screen.addstr(16,5,'PHASE A CURRENT: ')
    screen.addstr(17,5,'PHASE B CURRENT: ')
    screen.addstr(18,5,'PHASE C CURRENT: ')
    screen.addstr(19,5,'DC BUS CURRENT: ')
    screen.addstr(20,5,'DC BUS VOLTAGE: ')
    screen.addstr(21,5,'OUTPUT VOLTAGE: ')
    screen.addstr(22,5,'PHASE AB VOLTAGE: ')
    screen.addstr(23,5,'PHASE BC VOLTAGE: ')
    screen.addstr(24,5,'VSM STATE: ')
    screen.addstr(25,5,'INVERTER STATE: ')
    screen.addstr(26,5,'INVERTER RUN MODE: ')
    screen.addstr(27,5,'INVERTER ACTIVE DISCHARGE STATE: ')
    screen.addstr(28,5,'INVERTER COMMAND MODE: ')
    screen.addstr(29,5,'INVERTER ENABLE: ')
    screen.addstr(30,5,'INVERTER LOCKOUT: ')
    screen.addstr(31,5,'DIRECTION COMMAND: ')
    screen.addstr(32,5,'POST FAULT LO: ')
    screen.addstr(33,5,'POST FAULT HI: ')
    screen.addstr(34,5,'RUN FAULT LO: ')
    screen.addstr(35,5,'RUN FAULT HI: ')
    screen.addstr(36,5,'COMMANDED TORQUE: ')
    screen.addstr(37,5,'TORQUE FEEDBACK: ')
    screen.addstr(38,5,'RMS UPTIME: ')
    screen.addstr(3,55,'THROTTLE CONTROL UNIT')
    screen.addstr(4,55,'TCU UPTIME: ')
    screen.addstr(5,55,'TCU STATE: ')
    screen.addstr(6,55,'START BUTTON ID: ')
    screen.addstr(3,105,'POWER CONTROL UNIT')
    screen.addstr(4,105,'PCU UPTIME: ')
    screen.addstr(5,105,'PCU STATE: ')
    screen.addstr(6,105,'BMS FAULT: ')
    screen.addstr(7,105,'IMD FAULT: ')
    curses.wrapper(live)
    curses.endwin()

def live(screen):
    ser = serial.Serial(sys.argv[1], 115200)
    incomingLine = ''
    screen.nodelay(True)
    char = screen.getch()
    while char != ord('q') and char != ord('Q'):
        char = screen.getch()
        data = ser.read()
        if len(data) > 0:
            incomingLine += data
            if ('\n' in incomingLine):
                updateScreen(screen, incomingLine[0:incomingLine.find('\n')])
                incomingLine = incomingLine[incomingLine.find('\n') + 1:]

def updateScreen(screen, incomingLine):
    if ('MODULE A TEMP' in incomingLine):
        clearLine(4,5)
        screen.addstr(4,5,incomingLine)
    if ('MODULE B TEMP' in incomingLine):
        clearLine(5,5)
        screen.addstr(5,5,incomingLine)
    if ('MODULE C TEMP' in incomingLine):
        clearLine(6,5)
        screen.addstr(6,5,incomingLine)
    if ('GATE DRIVER BOARD TEMP' in incomingLine):
        clearLine(7,5)
        screen.addstr(7,5,incomingLine)
    if ('RTD 4 TEMP' in incomingLine):
        clearLine(8,5)
        screen.addstr(8,5,incomingLine)
    if ('RTD 5 TEMP' in incomingLine):
        clearLine(9,5)
        screen.addstr(9,5,incomingLine)
    if ('MOTOR TEMP' in incomingLine):
        clearLine(10,5)
        screen.addstr(10,5,incomingLine)
    if ('TORQUE SHUDDER' in incomingLine):
        clearLine(11,5)
        screen.addstr(11,5,incomingLine)
    if ('MOTOR ANGLE' in incomingLine):
        clearLine(12,5)
        screen.addstr(12,5,incomingLine)
    if ('MOTOR SPEED' in incomingLine):
        clearLine(13,5)
        screen.addstr(13,5,incomingLine)
    if ('ELEC OUTPUT FREQ' in incomingLine):
        clearLine(14,5)
        screen.addstr(14,5,incomingLine)
    if ('DELTA RESOLVER FILT' in incomingLine):
        clearLine(15,5)
        screen.addstr(15,5,incomingLine)
    if ('PHASE A CURRENT' in incomingLine):
        clearLine(16,5)
        screen.addstr(16,5,incomingLine)
    if ('PHASE B CURRENT' in incomingLine):
        clearLine(17,5)
        screen.addstr(17,5,incomingLine)
    if ('PHASE C CURRENT' in incomingLine):
        clearLine(18,5)
        screen.addstr(18,5,incomingLine)
    if ('DC BUS CURRENT' in incomingLine):
        clearLine(19,5)
        screen.addstr(19,5,incomingLine)
    if ('DC BUS VOLTAGE' in incomingLine):
        clearLine(20,5)
        screen.addstr(20,5,incomingLine)
    if ('OUTPUT VOLTAGE' in incomingLine):
        clearLine(21,5)
        screen.addstr(21,5,incomingLine)
    if ('PHASE AB VOLTAGE' in incomingLine):
        clearLine(22,5)
        screen.addstr(22,5,incomingLine)
    if ('PHASE BC VOLTAGE' in incomingLine):
        clearLine(23,5)
        screen.addstr(23,5,incomingLine)
    if ('VSM STATE' in incomingLine):
        clearLine(24,5)
        screen.addstr(24,5,incomingLine)
    if ('INVERTER STATE' in incomingLine):
        clearLine(25,5)
        screen.addstr(25,5,incomingLine)
    if ('INVERTER RUN MODE' in incomingLine):
        clearLine(26,5)
        screen.addstr(26,5,incomingLine)
    if ('INVERTER ACTIVE DISCHARGE STATE' in incomingLine):
        clearLine(27,5)
        screen.addstr(27,5,incomingLine)
    if ('INVERTER COMMAND MODE' in incomingLine):
        clearLine(28,5)
        screen.addstr(28,5,incomingLine)
    if ('INVERTER ENABLE' in incomingLine):
        clearLine(29,5)
        screen.addstr(29,5,incomingLine)
    if ('INVERTER LOCKOUT' in incomingLine):
        clearLine(30,5)
        screen.addstr(30,5,incomingLine)
    if ('DIRECTION COMMAND' in incomingLine):
        clearLine(31,5)
        screen.addstr(31,5,incomingLine)
    if ('POST FAULT LO' in incomingLine):
        clearLine(32,5)
        screen.addstr(32,5,incomingLine)
    if ('POST FAULT HI' in incomingLine):
        clearLine(33,5)
        screen.addstr(33,5,incomingLine)
    if ('RUN FAULT LO' in incomingLine):
        clearLine(34,5)
        screen.addstr(34,5,incomingLine)
    if ('RUN FAULT HI' in incomingLine):
        clearLine(35,5)
        screen.addstr(35,5,incomingLine)
    if ('COMMANDED TORQUE' in incomingLine):
        clearLine(36,5)
        screen.addstr(36,5,incomingLine)
    if ('TORQUE FEEDBACK' in incomingLine):
        clearLine(37,5)
        screen.addstr(37,5,incomingLine)
    if ('RMS UPTIME' in incomingLine):
        clearLine(38,5)
        screen.addstr(38,5,incomingLine)
    if ('TCU UPTIME' in incomingLine):
        clearLine(4,55)
        screen.addstr(4,55,incomingLine)
    if ('TCU STATE' in incomingLine):
        clearLine(5,55)
        screen.addstr(5,55,incomingLine)
    if ('START BUTTON ID' in incomingLine):
        clearLine(6,55)
        screen.addstr(6,55,incomingLine)
    if ('PCU UPTIME' in incomingLine):
        clearLine(4,105)
        screen.addstr(4,105,incomingLine)
    if ('PCU STATE' in incomingLine):
        clearLine(5,105)
        screen.addstr(5,105,incomingLine)
    if ('BMS FAULT' in incomingLine):
        clearLine(6,105)
        screen.addstr(6,105,incomingLine)
    if ('IMD FAULT' in incomingLine):
        clearLine(7,105)
        screen.addstr(7,105,incomingLine)
    #todo finish
    screen.refresh()

def clearLine(y, x):
    blanks = '                                                  '
    screen.addstr(y,x,blanks);

main()
