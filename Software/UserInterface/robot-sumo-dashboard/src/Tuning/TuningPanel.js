import { React, useEffect } from "react";
import { Button, Slider, Stack } from '@mui/material';
import SendIcon from '@mui/icons-material/Send';

const TuningPanel = ({ onUserChangeTuningData, TuningData, onUserClickSend, onUserClickCalibration }) => {

    return (
        <div style={{ paddingTop: "40px", width: "40%", minWidth: "300px" }}>
            <Stack spacing={3} direction="column">
                <Stack spacing={2} direction="row" sx={{ mb: 1 }} alignItems="center">
                    <p>Kp</p>
                    <Slider
                        disabled={false}
                        marks={false}
                        step={0.1}
                        max={8000}
                        min={0}
                        value={TuningData.KP}
                        size="small"
                        valueLabelDisplay="on"
                        onChange={(e) => {
                            onUserChangeTuningData({ KP: e.target.value })
                        }}
                    />

                </Stack>

                <Stack spacing={2} direction="row" sx={{ mb: 1 }} alignItems="center">
                    <p>Kd</p>
                    <Slider
                        disabled={false}
                        marks={false}
                        step={0.01}
                        max={8000}
                        min={0}
                        size="small"
                        value={TuningData.KD}
                        valueLabelDisplay="on"
                        onChange={(e) => {
                            onUserChangeTuningData({ KD: e.target.value })
                        }}
                    />
                </Stack>
                <Stack spacing={2} direction="row" sx={{ mb: 1 }} alignItems="center">
                    <p>MaxSpeed</p>
                    <Slider
                        disabled={false}
                        marks={false}
                        step={1}
                        max={16384}
                        min={1}
                        value={TuningData.Max_Speed}
                        size="small"
                        valueLabelDisplay="on"
                        onChange={(e) => {
                            onUserChangeTuningData({ Max_Speed: e.target.value })
                        }} />
                </Stack>
            </Stack>

            <Button onClick={onUserClickSend} style={{ marginTop: "20px", float: "right" }} variant="contained" endIcon={<SendIcon />}>
                Send
            </Button>
            <Button onClick={onUserClickCalibration} style={{ marginTop: "20px", float: "left" }} variant="contained" endIcon={<SendIcon />}>
                Calibration
            </Button>
        </div>
    )
}

export default TuningPanel;