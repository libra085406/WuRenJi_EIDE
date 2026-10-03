#include "Int_MPU6050.h"

static uint8_t MPU6050_Raw_Data_Buffer[14];
static int16_t accel_x_avg = 0;
static int16_t accel_y_avg = 0;
static int16_t accel_z_avg = 0;
static int16_t gyro_x_avg = 0;
static int16_t gyro_y_avg = 0;
static int16_t gyro_z_avg = 0;

// 通过I2C对MPU6050内的某个寄存器进行写操作
void Int_MPU6050_WriteReg(uint8_t regAddr, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, (uint16_t)regAddr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

// 通过I2C对MPU6050内的寄存器进行连续读操作
void Int_MPU6050_ReadRegs(uint8_t regStartAddr, uint8_t *pBuff, uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, regStartAddr, I2C_MEMADD_SIZE_8BIT, pBuff, len, 1000);
}

void Int_MPU6050_Init(void)
{
    // 1. 让6050里的所有寄存器复位
    Int_MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x80);
    HAL_Delay(150);
    // 让 6050退出睡眠模式
    Int_MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x00);

    // 2. 让6050的陀螺仪量程为±2000dps
    Int_MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x18);
    // 3. 让6050的加速度量程为±2g
    Int_MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x00); // 其实可以不写 因为默认值符合需求

    // 4. 让6050的DLFP只开1档 如果开太高 会引入过高的延迟 不太好。
    Int_MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x01);

    // 5. 6050的采样率保持1khz 不分频
    Int_MPU6050_WriteReg(MPU6050_REG_SMPRT_DIV, 0x00); // 其实可以不写 因为默认值符合需求

    // 6. 关闭所有中断
    Int_MPU6050_WriteReg(MPU6050_REG_INT_ENABLE, 0x00); // 其实可以不写 因为默认值符合需求

    // 7. 关闭FIFO使能 关闭主I2C
    Int_MPU6050_WriteReg(MPU6050_REG_USER_CTRL, 0x00); // 其实可以不写 因为默认值符合需求

    // 8. 关闭所有的FIFO的子开关
    Int_MPU6050_WriteReg(MPU6050_REG_FIFO_EN, 0x00); // 其实可以不写 因为默认值符合需求

    // 9. 选择了X轴的机械时钟源
    Int_MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01);

    // 10. 6个轴 都不需要待机
    Int_MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_2, 0x00); // 其实可以不写 因为默认值符合需求

    Int_MPU6050_Calibration();
}

void Int_MPU6050_ReadDataRAW(ACCEL_t *accel, GYRO_t *gyro)
{
    Int_MPU6050_ReadRegs(MPU6050_REG_ACCEL_XOUT_H, MPU6050_Raw_Data_Buffer, 14);

    accel->x = (MPU6050_Raw_Data_Buffer[0] << 8) | MPU6050_Raw_Data_Buffer[1];
    accel->y = (MPU6050_Raw_Data_Buffer[2] << 8) | MPU6050_Raw_Data_Buffer[3];
    accel->z = (MPU6050_Raw_Data_Buffer[4] << 8) | MPU6050_Raw_Data_Buffer[5];

    gyro->x = (MPU6050_Raw_Data_Buffer[8] << 8) | MPU6050_Raw_Data_Buffer[9];
    gyro->y = (MPU6050_Raw_Data_Buffer[10] << 8) | MPU6050_Raw_Data_Buffer[11];
    gyro->z = (MPU6050_Raw_Data_Buffer[12] << 8) | MPU6050_Raw_Data_Buffer[13];
}

void Int_MPU6050_Calibration(void)
{
    // 1. 先判断是否稳定（以角速度为标准 判断是否稳定）
    // 当前的角速度 减去上次的角速度 小于50 就让计数值++一次 计数值==400就稳定了
    GYRO_t last_gyro = {0};
    ACCEL_t current_accel = {0};
    GYRO_t current_gyro = {0};

    uint16_t count = 0;
    int32_t accel_x_sum = 0;
    int32_t accel_y_sum = 0;
    int32_t accel_z_sum = 0;
    int32_t gyro_x_sum = 0;
    int32_t gyro_y_sum = 0;
    int32_t gyro_z_sum = 0;
    while (1)
    {

        Int_MPU6050_ReadDataRAW(&current_accel, &current_gyro);
        if ((abs(current_gyro.x - last_gyro.x) < 50) && (abs(current_gyro.y - last_gyro.y) < 50) && (abs(current_gyro.z - last_gyro.z) < 50))
        {
            count++;
            gyro_x_sum += current_gyro.x;
            gyro_y_sum += current_gyro.y;
            gyro_z_sum += current_gyro.z;
            accel_x_sum += current_accel.x;
            accel_y_sum += current_accel.y;
            accel_z_sum += current_accel.z;
        }
        else
        {
            count = 0;
            accel_x_sum = 0;
            accel_y_sum = 0;
            accel_z_sum = 0;
            gyro_x_sum = 0;
            gyro_y_sum = 0;
            gyro_z_sum = 0;
        }
        last_gyro = current_gyro;
        if (count == 400)
        {
            accel_x_avg = accel_x_sum / 400;
            accel_y_avg = accel_y_sum / 400;
            accel_z_avg = accel_z_sum / 400;
            gyro_x_avg = gyro_x_sum / 400;
            gyro_y_avg = gyro_y_sum / 400;
            gyro_z_avg = gyro_z_sum / 400;

            break;
        }
        HAL_Delay(1);
    }
}

void Int_MPU6050_ReadDataByCalibration(ACCEL_t *accel, GYRO_t *gyro)
{
    Int_MPU6050_ReadDataRAW(accel, gyro);

    // 直接去做减去平均值的操作
    accel->x -= accel_x_avg;
    accel->y -= accel_y_avg;
    accel->z -= accel_z_avg;
    gyro->x -= gyro_x_avg;
    gyro->y -= gyro_y_avg;
    gyro->z -= gyro_z_avg;
}
