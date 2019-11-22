byte[4] transition (byte[4] startColor, byte[4] endColor, float currTime, float duration, bool complete) {
    float progress = currTime / duration;
    if (progress < 1.0) {
        complete = true;
        return endColor;
    }
    output = byte[4];
    output[0] = startColor[0] + (endColor[0] - startColor[0]) * progress;
    output[1] = startColor[1] + (endColor[1] - startColor[1]) * progress;
    output[2] = startColor[2] + (endColor[2] - startColor[2]) * progress;
    output[3] = startColor[3] + (endColor[3] - startColor[3]) * progress;
    return output;
}