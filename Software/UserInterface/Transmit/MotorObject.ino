

void TuneSpeed(int Value, byte ENABLE_PIN, byte BRAKE_PIN, byte DIR_PIN, Tone *MotorFrequencyTuning)
{
    digitalWrite(ENABLE_PIN, HIGH);
    digitalWrite(BRAKE_PIN, HIGH);

    digitalWrite(DIR_PIN, Value < 0 ? LOW : HIGH);

    if (Value != 0)
    {
        &MotorFrequencyTuning.play(map(abs(Value), 1, 1024, 3000, 26000));
    }
    else
    {
        &MotorFrequencyTuning.play(0);
        digitalWrite(BRAKE_PIN, LOW);
    }
}
