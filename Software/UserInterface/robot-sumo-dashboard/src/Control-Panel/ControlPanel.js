import { ListItemIcon, ListItem, Button, List, ListItemButton, ListItemText, ListSubheader, ThemeProvider, createTheme } from "@mui/material";
import React from "react";
import HandymanTwoToneIcon from '@mui/icons-material/HandymanTwoTone';

import "./ControlPanel.css"



const theme = createTheme({

    palette: {
        primary: {
            main: '#FEB130',
        },
        secondary: {
            main: '#ff80ab',
        },
    },
})

const ControlPanel = ({ ModeSelected, ChangeMode }) => {
    const Feature = ["Debug", "Driving", "Tuning"]
    return (
        <List className="Navigator" variant="outlined">
            <ListItem ></ListItem>
            {Feature.map((i) => {
                return (
                    <ListItem key={i} style={{ "width": "fill-available" }}>
                        <ListItemButton onClick={() => {
                            ChangeMode(i);
                        }}
                            style={{ backgroundColor: ModeSelected == i ? "#de6b28" : "" }}
                            href="#simple-list" >
                            <ListItemText color="primary" primary={i} />
                        </ListItemButton>
                    </ListItem>
                )
            })}
        </List >

    );
}

export default ControlPanel