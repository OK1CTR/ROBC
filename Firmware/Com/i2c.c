// Example: Write one byte to a register
/*
void I2C_Write(I2C_TypeDef *I2Cx, uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    // 1. Generate Start
    LL_I2C_GenerateStartCondition(I2Cx);
    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    // 2. Send Slave Address (Write mode)
    LL_I2C_TransmitAddress8Bit(I2Cx, devAddr);
    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
    LL_I2C_ClearFlag_ADDR(I2Cx);
    while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

    // 3. Send Register Address
    LL_I2C_TransmitData8(I2Cx, regAddr);
    while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

    // 4. Send Data
    LL_I2C_TransmitData8(I2Cx, data);
    while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

    // 5. Generate Stop
    LL_I2C_GenerateStopCondition(I2Cx);
}
*/
