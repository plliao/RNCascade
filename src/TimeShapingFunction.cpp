#include <TimeShapingFunction.h>

TFlt EXPShapingFunction::Value(TFlt srcTime,TFlt dstTime) const {
   if (srcTime < dstTime) return 1.0;
   else return 0.0;
}

TFlt EXPShapingFunction::Integral(TFlt srcTime,TFlt dstTime) const {
   if (srcTime < dstTime) return dstTime - srcTime;
   else return 0.0;
}

bool EXPShapingFunction::Before(TFlt srcTime,TFlt dstTime) const {
   return srcTime < dstTime;
}

TFlt POWShapingFunction::Value(TFlt srcTime,TFlt dstTime) const {
   if (Before(srcTime, dstTime)) return 1.0 / (dstTime - srcTime);
   else return 0.0;
}

TFlt POWShapingFunction::Integral(TFlt srcTime,TFlt dstTime) const {
   if (Before(srcTime, dstTime)) return TMath::Log((dstTime - srcTime) / delta);
   else return 0.0;
}

bool POWShapingFunction::Before(TFlt srcTime,TFlt dstTime) const {
   return (srcTime + delta) < dstTime;
}

TFlt RAYShapingFunction::Value(TFlt srcTime,TFlt dstTime) const {
   if (srcTime < dstTime) return dstTime - srcTime;
   else return 0.0;
}

TFlt RAYShapingFunction::Integral(TFlt srcTime,TFlt dstTime) const {
   if (srcTime < dstTime) return TMath::Power(dstTime - srcTime, 2.0) / 2.0;
   else return 0.0;
}

bool RAYShapingFunction::Before(TFlt srcTime,TFlt dstTime) const {
   return srcTime < dstTime;
}