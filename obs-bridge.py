from obswebsocket import obsws, requests
import rtmidi

host = "localhost"
port = 4455

ws = obsws(host, port)
ws.connect()

midiin = rtmidi.MidiIn()
for i in range(midiin.get_port_count()):
    print(i, midiin.get_port_name(i))

port = midiin.open_port(2)

while True:
    m_data = midiin.get_message()
    if m_data:
        m, delta_time = m_data
        controller_number = m[1]
        controller_value = m[2]
        channel = m[0] & 0x0F

        val = controller_value / 127.0
        if channel == 0:
            ws.call(
                requests.SetInputVolume(inputName="SPOTIFY AUDIO", inputVolumeMul=val)
            )
            print(f"SETTING SPOTIFY TO {val}")

        if channel == 1:
            ws.call(requests.SetInputVolume(inputName="MIC AUDIO", inputVolumeMul=val))
            print(f"SETTING MICROPHONE TO {val}")
