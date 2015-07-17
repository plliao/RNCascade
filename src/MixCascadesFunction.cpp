#include <MixCascadesFunction.h>

TFlt MixCascadesFunction::JointLikelihood(Datum datum, TInt latentVariable) const {
   TFlt logP = -1 * kAlphas.GetDat(latentVariable).loss(datum);
   TFlt logPi = TMath::Log(parameter.kPi.GetDat(latentVariable));
   //printf("logP: %f, logPi=%f\n",logP(),logPi());
   return logP + logPi;
}

MixCascadesParameter& MixCascadesFunction::gradient(Datum datum) {
   parameterGrad.reset();

   for (THash<TInt,AdditiveRiskFunction>::TIter AI = kAlphas.BegI(); !AI.IsEnd(); AI++) {
      TInt key = AI.GetKey();
      AdditiveRiskParameter& alphas = AI.GetDat().gradient(datum);
      alphas *= latentDistributions.GetDat(datum.index).GetDat(key);
      //printf("index:%d, k:%d, latent distribution:%f\n",datum.index(), key(), latentDistributions.GetDat(datum.index).GetDat(key)());     
      parameterGrad.kPi.GetDat(key) = latentDistributions.GetDat(datum.index).GetDat(key);
      parameterGrad.kPi_times.GetDat(key)++; 
   }
   return parameterGrad;
}

void MixCascadesFunction::maximize() {
   for (THash<TInt,TFlt>::TIter PI = parameter.kPi_times.BegI(); !PI.IsEnd(); PI++) {
      PI.GetDat() = 0.0;
   }
}

void MixCascadesFunction::set(MixCascadesFunctionConfigure configure) {
   for (TInt i=0;i<configure.latentVariableSize;i++) {
      kAlphas.AddDat(i,AdditiveRiskFunction());
      kAlphas.GetDat(i).set(configure.configure);
      kAlphas.GetDat(i).observedWindow = configure.observedWindow;
   }
   parameter.init(configure.latentVariableSize, &kAlphas);
   parameterGrad.init(configure.latentVariableSize, &kAlphas);
}

void MixCascadesParameter::init(TInt latentVariableSize, THash<TInt,AdditiveRiskFunction>* KAlphasP) {
   kAlphasP = KAlphasP;
   TRnd rnd; rnd.PutSeed(time(NULL));
   for (TInt i=0;i<latentVariableSize;i++) {
      kPi.AddDat(i, rnd.GetUniDev() * 1.0 + 1.0);
      kPi_times.AddDat(i,0.0);
   }
   TFlt sum = 0.0;
   for (TInt i=0;i<latentVariableSize;i++) sum += kPi.GetDat(i);
   for (TInt i=0;i<latentVariableSize;i++) kPi.GetDat(i) /= sum;
}

void MixCascadesParameter::set(MixCascadesFunctionConfigure configure) {
   for (THash<TInt,AdditiveRiskFunction>::TIter AI = kAlphas.BegI(); !AI.IsEnd(); AI++) {
      AdditiveRiskFunction& f = AI.GetDat();
      f.set(configure.configure);
   }
}

void MixCascadesParameter::reset() {
   for (THash<TInt,TFlt>::TIter piI = kPi.BegI(); !piI.IsEnd(); piI++) { 
      piI.GetDat() = 0.0;
      kPi_times.GetDat(piI.GetKey()) = 0.0;
   }
}

MixCascadesParameter& MixCascadesParameter::operator = (const MixCascadesParameter& p) {
   kAlphas.Clr();
   kPi.Clr();
   kPi_times.Clr();
   kAlphasP = p.kAlphasP;
   for(THash<TInt,AdditiveRiskFunction>::TIter AI = p.kAlphas.BegI(); !AI.IsEnd(); AI++) {
      TInt key = AI.GetKey();
      kAlphas.AddDat(key,AI.GetDat());   
      kPi.AddDat(key,p.kPi.GetDat(key));
      kPi_times.AddDat(key,p.kPi_times.GetDat(key));
   }
   return *this;
}

MixCascadesParameter& MixCascadesParameter::operator += (const MixCascadesParameter& p) {
   for(THash<TInt,AdditiveRiskFunction>::TIter AI = p.kAlphasP->BegI(); !AI.IsEnd(); AI++) {
      TInt key = AI.GetKey();
      if (!kAlphas.IsKey(key)) {
         kAlphas.AddDat(key,AdditiveRiskFunction());
         kPi.AddDat(key,0.0);
         kPi_times.AddDat(key,0.0);
      }
      kAlphas.GetDat(key).getParameter() += AI.GetDat().getParameterGrad();
      kPi.GetDat(key) += p.kPi.GetDat(key);
      kPi_times.GetDat(key) += p.kPi_times.GetDat(key);
   }
   return *this;
}

MixCascadesParameter& MixCascadesParameter::operator *= (const TFlt multiplier) {
   for(THash<TInt,AdditiveRiskFunction>::TIter AI = kAlphas.BegI(); !AI.IsEnd(); AI++) {
      AI.GetDat().getParameter() *= multiplier;
   }
   return *this;
}

MixCascadesParameter& MixCascadesParameter::projectedlyUpdateGradient(const MixCascadesParameter& p) {
   for(THash<TInt,AdditiveRiskFunction>::TIter AI = p.kAlphas.BegI(); !AI.IsEnd(); AI++) {
      TInt key = AI.GetKey();
      kAlphasP->GetDat(key).getParameter().projectedlyUpdateGradient(AI.GetDat().getParameter());

      TFlt old = kPi.GetDat(key) * kPi_times.GetDat(key);
      kPi_times.GetDat(key) += p.kPi_times.GetDat(key);
      kPi.GetDat(key) = (old + p.kPi.GetDat(key))/kPi_times.GetDat(key);
   }
   return *this;
}
