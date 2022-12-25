%
%   Test STM32F4 implementations
%   (new interface)
%
%   COMMAND BYTE:
%   0x10 indicates given plaintext masks off
%   0x11 indicates given plaintext masks on
%   0x20 indicates fixed vs random test masks off
%   0x21 indicates fixed vs random test masks on
%   0x30 indicates fixed vs fixed test masks off
%   0x31 indicates fixed vs fixed test masks on
%
%   PAYLOAD:
%   * If 0x10 or 0x11
%       -> 2 bytes [#iterations]
%       -> 16 bytes [AES key]
%       -> 16 bytes [AES plaintext]
%       <- 16 bytes [last ciphertext]  
%   * If 0x20 or 0x21
%       -> 2 bytes [#iterations]
%       -> 16 bytes [AES key]
%       -> 16 bytes [fixed AES plaintext]
%       -> 16 bytes [random AES plaintext]
%       <- X bytes [coin flips, length #iterations]  
%   * If 0x30 or 0x31
%       -> 2 bytes [#iterations]
%       -> 16 bytes [AES key]
%       -> 16 bytes [fixed AES plaintext 1]
%       -> 16 bytes [fixed AES plaintext 2]
%       <- X bytes [coin flips, length #iterations]  

instrfind
if(~isempty(ans))
    fclose(ans)
    delete(ans)
end
clear all
clc

% Construct the serial object
baud = 9600;
%s = serial('/dev/ttyUSB0');
s = serial('COM3');
set(s,'baudrate',baud);
set(s,'parity','none');
set(s,'DataBits',8);
set(s,'StopBits',1);

% Number of executions
nr_of_meas = 1;
disp(['We will do ',num2str(nr_of_meas),' executions']);

% Test vectors
aes_key = uint8(hex2dec(['2b';'7e';'15';'16';'28';'ae';'d2';'a6';'ab';'f7';'15';'88';'09';'cf';'4f';'3c']));
aes_pt = uint8(hex2dec(['32';'43';'f6';'a8';'88';'5a';'30';'8d';'31';'31';'98';'a2';'e0';'37';'07';'34']));
aes_fix1 = uint8(hex2dec(['e1';'27';'ba';'9a';'55';'90';'d0';'cc';'3a';'3f';'32';'97';'e4';'05';'15';'f2']));

% Test commands
COMMAND = hex2dec('20');    % supported: 0x10, 0x11, 0x20, 0x21, 0x30, 0x31
ITERATIONS = 1;             % supported up to 4096

% Input/Output arrays
if (COMMAND == hex2dec('10') || COMMAND == hex2dec('11'))
    input(1:35) = uint8(0);
    output(1:16) = uint8(0);
else
    input(1:51) = uint8(0);
    output(1:ITERATIONS) = uint8(0);
    s.InputBufferSize = ITERATIONS;
end

set(s,'Timeout',0.007 * ITERATIONS);
fopen(s);

% The measurement loop
for index = 1:1:nr_of_meas

    % Fill Input array
    input(1) = uint8(COMMAND);
    input(2) = uint8(bitand(bitshift(ITERATIONS,-8),255));
    input(3) = uint8(bitand(ITERATIONS,255));
    if (COMMAND == hex2dec('10') || COMMAND == hex2dec('11'))
        input(4:19) = aes_key(1:16);
        input(20:35) = aes_pt(1:16);
    else
        input(4:19) = aes_key(1:16);
        input(20:35) = aes_pt(1:16);
        if (COMMAND == hex2dec('20') || COMMAND == hex2dec('21'))
            input(36:51) = uint8(floor(rand(1,16).*256));
        else
            input(36:51) = aes_fix1(1:16);
        end
    end

    % Write Input array
    fwrite(s,input,'uint8');
    a = reshape(dec2hex(input)',1,2*length(input));
    disp('--------------------------------------------------------');
    disp(sprintf('Measurement: %d',index));
    disp('INPUT:  '); disp(a); 
    
    %pause(30);
    
    % Fetch Output
    if (COMMAND == hex2dec('10') || COMMAND == hex2dec('11'))
        output = fread(s,16,'uint8');
        a = reshape(dec2hex(output)',1,2*length(output));
        %disp('--------------------------------------------------------');
        disp('OUTPUT : '); disp(a);
    else
        output = fread(s,ITERATIONS,'uint8');
        disp('OUTPUT : '); disp(output');
    end
    
    
    % Check correctness
    if (COMMAND == hex2dec('10') || COMMAND == hex2dec('11'))
        aes_in = aes_pt'; 
        for k=1:1:ITERATIONS
            aes_out = aes128_enc_mex(aes_in,aes_key');
            aes_in = aes_out;
        end
        
        if (isequal(output(1:16)',aes_out(1:16)) == 0)
            disp('error, result does not match!');
            fclose(s);
            delete(s);
            return;
        end
    end
end

disp('--------------------------------------------------------');
fclose(s);
delete(s);
