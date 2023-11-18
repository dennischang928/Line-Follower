import React, { useState, useEffect, useRef } from 'react';
import Button from '@mui/material/Button';
import ControlPanel from './Control-Panel/ControlPanel';
import TuningPanel from './Tuning/TuningPanel';
import { BrowserSerial } from "browser-serial";


const serial = new BrowserSerial({ serialOptions: { baudRate: 115200 } });

const UserInterface = () => {
    let ConnectedPortWriter;
    let writer
    const [IsConnected, setIsConnected] = useState(false);
    const [ControlMode, setControlMode] = useState("");
    const [TuningData, setTuningData] = useState({
        KP: 1,
        KD: 1,
        Max_Speed: 1,
    });

    const d = useRef(ControlMode);

    // const handleKeyPress = (event) => {
    //     CommandHandler(event.code)
    // }

    useEffect(() => {
        document.addEventListener('keydown', handleKeyPress, true)
    }, [])

    useEffect(() => {
        d.current = ControlMode;
    }, [ControlMode]);

    useEffect(() => {
        console.log(IsConnected); // Log whenever IsConnected changes
    }, [IsConnected]);


    const ChangeMode = ({ ModeSelected }) => {
        setControlMode(ModeSelected);
    }



    const onUserClickSend = () => {
        TuningCommanderHandler({ Mode: ControlMode, TuningData: TuningData })
    }

    const TuningCommanderHandler = ({ Mode, TuningData }) => {
        if (Mode == "Tuning") {
            serial.write(`TP${TuningData.KP}D${TuningData.KD}MxSP${TuningData.Max_Speed}`)
        }
    }

    const handleKeyPress = (keypressed) => {
        DrivingCommandHandler({ Mode: d.current, PressedKeyCode: keypressed.code })
    }


    const onUserChangeTuningData = ({ KP, KD, Max_Speed }) => {
        setTuningData({
            ...TuningData,
            KP: KP ? KP : TuningData.KP,
            KD: KD ? KD : TuningData.KD,
            Max_Speed: Max_Speed ? Max_Speed : TuningData.Max_Speed,
        })
    }

    const DrivingCommandHandler = ({ Mode, PressedKeyCode }) => {
        let MessageToSend = ''
        if (Mode == 'Driving') { // Driving Mode
            switch (PressedKeyCode) {
                case 'KeyA':
                    MessageToSend = 'L';
                    break;
                case 'KeyD':
                    MessageToSend = 'R';
                    break;
                case 'KeyS':
                    MessageToSend = 'B';
                    break;
                case 'KeyW':
                    MessageToSend = 'F';
                    break;
                case 'Space':
                    MessageToSend = 'S';
                    break;
                default:
                    break;
            }
            if (MessageToSend) {
                console.log(`${Mode[0]}${MessageToSend}`)
                serial.write(`${Mode[0]}${MessageToSend}`);
            }
        }
    }

    const onUserClickCalibration = () => {
        serial.write("C")
    }

    const OnUserClickConnect = async () => {
        if (IsConnected == false) {
            try {
                await serial.connect().then(() => {
                    setIsConnected(true)
                })
            }
            catch (err) { console.log(err) }
        } else {
            try {
                await serial.disconnect().then(() => {
                    setIsConnected(false)
                })
            } catch (e) {
            }
        }
    }



    const TuningPanelRender = () => {
        if (ControlMode == "Tuning") {
            return <TuningPanel
                onUserChangeTuningData={onUserChangeTuningData} TuningData={TuningData}
                onUserClickSend={onUserClickSend} onUserClickCalibration={onUserClickCalibration} />
        }
    }

    return (
        <div style={{ display: "flex" }}>
            <ControlPanel ModeSelecxted={ControlMode} ChangeMode={ChangeMode} />
            <div style={{ paddingLeft: "2vw", width: "100%" }}>
                <h2>Line Follower Console</h2>
                <Button onClick={OnUserClickConnect} variant={IsConnected ? "contained" : "outlined"}>{IsConnected ? "Click to Disconnect" : "Click to Connect Device"}</Button>
                {TuningPanelRender()}
            </div>
        </div >
    );
}

export default UserInterface