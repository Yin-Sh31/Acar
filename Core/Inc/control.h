#define aleft -1.5 // 直角左转
#define left -1.0  // 左转
#define mleft -0.5 // 小左转
#define mringt 0.5 // 小右转
#define ringt 1.0  // 右转
#define aright 1.5 // 直角右转

const float kp = 30;  // 偏转灵敏度
const float ki = 0.5; // 回调
const float kd = 2;   // 过偏抵消
#define be_speed 600  // 基础速度
#define sl 8          // 暂停检测时间 sl*cir
#define cir 10        // 控制周期
