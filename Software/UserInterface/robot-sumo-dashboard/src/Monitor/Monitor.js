import { Slider } from "@mui/material";
import React from "react";
import "./Monitor.css"

const Monitor = ({ Error }) => {

    return (<div className="Monitor">
        <Slider
            track={false}
            disabled={false}
            marks={false}
            step={0.1}
            max={7}
            min={-7}
            value={Error}
            disabled={true}
            valueLabelDisplay="on"
        />
    </div>)
}

export default Monitor;