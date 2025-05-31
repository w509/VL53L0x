while (vl53l0x_init(&vl53l0x_dev))//vl53l0x初始化
  HAL_Delay(500);
vl53l0x_set_mode(&vl53l0x_dev, 0);//设置模式

while (1)
{
  uint16_t dis = vl53l0x_distance(&vl53l0x_dev); //获取距离,单位mm
}