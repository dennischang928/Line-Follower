import React, { useState, useEffect, useRef } from 'react';
import Button from '@mui/material/Button';
import Switch from '@mui/material/Switch';

import ControlPanel from './Control-Panel/ControlPanel';
import TuningPanel from './Tuning/TuningPanel';
import { BrowserSerial } from "browser-serial";
import Monitor from './Monitor/Monitor';
import CloseFullscreenIcon from '@mui/icons-material/CloseFullscreen';
import { FormControl, FormControlLabel, FormGroup, IconButton } from '@mui/material';


const serial = new BrowserSerial({ serialOptions: { baudRate: 115200 } });


const UserInterface = () => {
    let ConnectedPortWriter;
    let writer
    const [IsConnected, setIsConnected] = useState(false);
    const [ControlMode, setControlMode] = useState("");
    const [TuningData, setTuningData] = useState({
        KP: 100,
        KD: 0,
        Max_Speed: 1000,
    });
    const [Error, setError] = useState(0);
    const [IsDebugModeOn, setIsDebugModeOn] = useState(false);
    const [IsPIDLoopStart, setIsPIDLoopStart] = useState(false);
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
        console.log(IsConnected); // Log whenever IsConnected changesr
    }, [IsConnected]);



    const ChangeMode = (ModeSelected) => {
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
                    serial.readLoop(OnSerialReceive)
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



    const OnSerialReceive = (e) => {
        console.log(e)
        setError(e);
    }

    const TuningPanelRender = () => {
        if (ControlMode == "Tuning") {
            return <TuningPanel
                onUserChangeTuningData={onUserChangeTuningData} TuningData={TuningData}
                onUserClickSend={onUserClickSend} onUserClickCalibration={onUserClickCalibration} />
        }
    }

    const MonitorRender = () => {
        if (IsDebugModeOn) {
            return <Monitor Error={Error} />
        }
    }

    const OnUserToggleDebugMode = () => {
        setIsDebugModeOn(IsDebugModeOn => !IsDebugModeOn)
    }

    const OnUserClickPIDStart = () => {
        setIsPIDLoopStart(IsPIDLoopStart => !IsPIDLoopStart)
    }



    useEffect(() => { serial.write(`B${IsDebugModeOn ? 'E' : 'D'}`) }, [IsDebugModeOn])
    useEffect(() => { serial.write(`K${IsPIDLoopStart ? 'E' : 'D'}`) }, [IsPIDLoopStart])

    return (
        <div style={{ display: "flex" }}>
            <ControlPanel ModeSelected={ControlMode} ChangeMode={ChangeMode} />
            <div style={{ paddingLeft: "2vw", width: "100%" }}>
                <h2>Line Follower Console</h2>
                <div style={{ display: 'flex' }}>
                    <Button onClick={OnUserClickConnect} variant={IsConnected ? "contained" : "outlined"}>{IsConnected ? "Click to Disconnect" : "Click to Connect Device"}</Button>
                    <FormGroup style={{ paddingLeft: "10px" }}>
                        <FormControlLabel control={<Switch checked={IsDebugModeOn} onChange={OnUserToggleDebugMode} name="DebugMode" />} label="Debug" />
                        <FormControlLabel control={<Switch checked={IsPIDLoopStart} onChange={OnUserClickPIDStart} name="DebugMode" />} label="Start Line Following" />
                    </FormGroup>
                </div>
                {TuningPanelRender()}
                {MonitorRender()}
            </div>
        </div >
    );
}

export default UserInterface