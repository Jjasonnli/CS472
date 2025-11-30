# Q1 Time Travel Debugging
1. laptop might be on "sleep" mode. 
    a) I'll reboot the laptop.
    b) I'll compare with other "good" servers
2. Bad or unsynced NTP server
    a) compare with other servers
    b) look at reference_id
3. Client math error
    a) review how I calculate current_ntp_time
    b) cross check with other servers

# Q2 Network distance detective work
=== NTP Time Synchronization Results ===
Server: pool.ntp.org
Server Time: 2025-10-14 00:52:39.743674 (local time)
Local Time:  2025-10-14 00:52:39.737775 (local time)
Round Trip Delay: 0.026620

Time Offset: 0.019208 seconds
Final dispersion 0.013310

Your clock is running BEHIND by 19.21ms
Your estimated time error will be +/- 13.31ms

=== NTP Time Synchronization Results ===
Server: time.nist.gov
Server Time: 2025-10-14 00:52:24.028263 (local time)
Local Time:  2025-10-14 00:52:24.030878 (local time)
Round Trip Delay: 0.078649

Time Offset: 0.036709 seconds
Final dispersion 0.039325

Your clock is running BEHIND by 36.71ms
Your estimated time error will be +/- 39.32ms

# answer
pool.ntp.org server is closer to me which makes the estimated time error shorter than the time.nist.gov server.
Physical distance matters because latency grows with distance. A farther server also yields a wider error bar even if it's "better". 
A "worse but closer" server can be more accurate for me because of the network path. The network path along with the server quality set the effective accuracy. pool = +/-13.31 ms whereas nist = +/-39.32ms.

# Below are my different outputs for clients

jasonli@n3-29-42 01-ntp % ./ntp-client
Querying NTP server: pool.ntp.org
Server IP: 72.30.35.89
Connecting to pool.ntp.org (72.30.35.89) on port 123
DEBUG: li_vn_mode byte = 0xE3
  Leap Indicator = 3
  Version = 4
  Mode = 3
  Binary breakdown: LI=11 VN=100 Mode=011

Sending NTP request...
print_ntp_packet_info() - Request Packet
--- Request Packet ---
Leap Indicator: 3
Version: 4
Mode: 3
Stratum: 0
Poll: 6
Precision: -20
Reference ID: [0x00000000] NONE
Root Delay: 0
Root Dispersion: 0
Reference Time: INVALID_TIME (Local Time)
Original Time (T1): INVALID_TIME (Local Time)
Receive Time (T2): INVALID_TIME (Local Time)
Transmit Time (T3): 2025-10-14 00:51:54.091271 (Local Time)

Received NTP response from pool.ntp.org!
print_ntp_packet_info() - Response Packet
--- Response Packet ---
Leap Indicator: 0
Version: 4
Mode: 4
Stratum: 2
Poll: 6
Precision: -25
Reference ID: [0x628b853e] 98.139.133.62
Root Delay: 12
Root Dispersion: 22
Reference Time: 2025-10-14 00:46:50.731635 (Local Time)
Original Time (T1): 2025-10-14 00:51:54.091271 (Local Time)
Receive Time (T2): 2025-10-14 00:51:54.125576 (Local Time)
Transmit Time (T3): 2025-10-14 00:51:54.125586 (Local Time)

=== NTP Time Synchronization Results ===
Server: pool.ntp.org
Server Time: 2025-10-14 00:51:54.125586 (local time)
Local Time:  2025-10-14 00:51:54.112609 (local time)
Round Trip Delay: 0.021328

Time Offset: 0.023641 seconds
Final dispersion 0.010664

Your clock is running BEHIND by 23.64ms
Your estimated time error will be +/- 10.66ms

# Below ./ntp-client -s time.nist.gov

jasonli@n3-29-42 01-ntp % ./ntp-client -s time.nist.gov
Querying NTP server: time.nist.gov
Server IP: 132.163.97.6
Connecting to time.nist.gov (132.163.97.6) on port 123
DEBUG: li_vn_mode byte = 0xE3
  Leap Indicator = 3
  Version = 4
  Mode = 3
  Binary breakdown: LI=11 VN=100 Mode=011

Sending NTP request...
print_ntp_packet_info() - Request Packet
--- Request Packet ---
Leap Indicator: 3
Version: 4
Mode: 3
Stratum: 0
Poll: 6
Precision: -20
Reference ID: [0x00000000] NONE
Root Delay: 0
Root Dispersion: 0
Reference Time: INVALID_TIME (Local Time)
Original Time (T1): INVALID_TIME (Local Time)
Receive Time (T2): INVALID_TIME (Local Time)
Transmit Time (T3): 2025-10-14 00:52:23.952227 (Local Time)

Received NTP response from time.nist.gov!
print_ntp_packet_info() - Response Packet
--- Response Packet ---
Leap Indicator: 0
Version: 3
Mode: 4
Stratum: 1
Poll: 13
Precision: -29
Reference ID: [0x4e495354] NIST
Root Delay: 16
Root Dispersion: 32
Reference Time: 2025-10-14 00:52:16.000000 (Local Time)
Original Time (T1): 2025-10-14 00:52:23.952227 (Local Time)
Receive Time (T2): 2025-10-14 00:52:24.028261 (Local Time)
Transmit Time (T3): 2025-10-14 00:52:24.028263 (Local Time)

=== NTP Time Synchronization Results ===
Server: time.nist.gov
Server Time: 2025-10-14 00:52:24.028263 (local time)
Local Time:  2025-10-14 00:52:24.030878 (local time)
Round Trip Delay: 0.078649

Time Offset: 0.036709 seconds
Final dispersion 0.039325

Your clock is running BEHIND by 36.71ms
Your estimated time error will be +/- 39.32ms

# Below ./ntp-client -s pool.ntp.org
jasonli@n3-29-42 01-ntp % ./ntp-client -s pool.ntp.org
Querying NTP server: pool.ntp.org
Server IP: 192.155.94.72
Connecting to pool.ntp.org (192.155.94.72) on port 123
DEBUG: li_vn_mode byte = 0xE3
  Leap Indicator = 3
  Version = 4
  Mode = 3
  Binary breakdown: LI=11 VN=100 Mode=011

Sending NTP request...
print_ntp_packet_info() - Request Packet
--- Request Packet ---
Leap Indicator: 3
Version: 4
Mode: 3
Stratum: 0
Poll: 6
Precision: -20
Reference ID: [0x00000000] NONE
Root Delay: 0
Root Dispersion: 0
Reference Time: INVALID_TIME (Local Time)
Original Time (T1): INVALID_TIME (Local Time)
Receive Time (T2): INVALID_TIME (Local Time)
Transmit Time (T3): 2025-10-14 00:52:39.711117 (Local Time)

Received NTP response from pool.ntp.org!
print_ntp_packet_info() - Response Packet
--- Response Packet ---
Leap Indicator: 0
Version: 4
Mode: 4
Stratum: 2
Poll: 6
Precision: -24
Reference ID: [0x82cff4f0] 130.207.244.240
Root Delay: 955
Root Dispersion: 1862
Reference Time: 2025-10-14 00:47:03.762343 (Local Time)
Original Time (T1): 2025-10-14 00:52:39.711117 (Local Time)
Receive Time (T2): 2025-10-14 00:52:39.743636 (Local Time)
Transmit Time (T3): 2025-10-14 00:52:39.743674 (Local Time)

=== NTP Time Synchronization Results ===
Server: pool.ntp.org
Server Time: 2025-10-14 00:52:39.743674 (local time)
Local Time:  2025-10-14 00:52:39.737775 (local time)
Round Trip Delay: 0.026620

Time Offset: 0.019208 seconds
Final dispersion 0.013310

Your clock is running BEHIND by 19.21ms
Your estimated time error will be +/- 13.31ms