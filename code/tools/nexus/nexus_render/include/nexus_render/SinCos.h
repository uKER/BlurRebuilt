
#pragma once

//
// Structures for extracting sine/cosine pairs and quads
//

class SinCosAlpha
{
    private:
    union
    {
        struct { F32 sin_alpha, cos_alpha; };
        //struct { U64 packed; };
    };
    
    public:
    SinCosAlpha( F32 alpha )
    {
        //packed = fast_sin_cos_alpha( alpha );
		sin_alpha = sinf(alpha);
		cos_alpha = cosf(alpha);
    }
    const F32 SinAlpha() const { return sin_alpha; }
    const F32 CosAlpha() const { return cos_alpha; }
};

class SinAlphaCosBeta
{
    private:
    union
    {
        struct { F32 sin_alpha, cos_beta; };
        //struct { U64 packed; };
    };
    
    public:
    SinAlphaCosBeta( F32 alpha, F32 beta )
    {
        //packed = fast_sin_alpha_cos_beta( alpha, beta );
		sin_alpha = sinf(alpha);
		cos_beta = cosf(beta);
    }
    const F32 SinAlpha() const { return sin_alpha; }
    const F32 CosBeta()  const { return cos_beta; }
};

class SinAlphaBeta
{
    private:
    union
    {
        struct { F32 sin_alpha, sin_beta; };
        //struct { U64 packed; };
    };
    
    public:
    SinAlphaBeta( F32 alpha, F32 beta )
    {
        //packed = fast_sin_alpha_beta( alpha, beta );
		sin_alpha = sinf(alpha);
		sin_beta = cosf(beta);
    }
    const F32 SinAlpha() const { return sin_alpha; }
    const F32 SinBeta()  const { return sin_beta; }
};

class CosAlphaBeta
{
    private:
    union
    {
        struct { F32 cos_alpha; F32 cos_beta; };
        //struct { U64 packed; };
    };
    
    public:
    CosAlphaBeta( F32 alpha, F32 beta )
    {
        //packed = fast_cos_alpha_beta( alpha, beta );
		cos_alpha = cosf(alpha);
		cos_beta = cosf(beta);
    }
    const F32 CosAlpha() const { return cos_alpha; }
    const F32 CosBeta()  const { return cos_beta; }
};

class SinAlphaCosBetaSinGammaCosDelta
{
    private:
    union
    {
        struct { F32 sin_alpha, cos_beta, sin_gamma, cos_delta; };
        //struct { U128 packed; };
    };
    
    public:
    SinAlphaCosBetaSinGammaCosDelta( F32 alpha, F32 beta, F32 gamma, F32 delta )
    {
        //packed = fast_sin_alpha_cos_beta_sin_gamma_cos_delta( alpha, beta, gamma, delta );
		sin_alpha = cosf(alpha);
		cos_beta = cosf(beta);
		sin_gamma = cosf(gamma);
		cos_delta = cosf(delta);
    }
    const F32 SinAlpha() const { return sin_alpha; }
    const F32 CosBeta()  const { return cos_beta; }
    const F32 SinGamma() const { return sin_gamma; }
    const F32 CosDelta() const { return cos_delta; }
};

class SinCosAlphaBeta
{
    private:
    union
    {
        struct { F32 sin_alpha, cos_alpha, sin_beta, cos_beta; };
        //struct { U128 packed; };
    };
    
    public:
    SinCosAlphaBeta( F32 alpha, F32 beta )
    {
        //packed = fast_sin_cos_alpha_beta( alpha, beta );
		sin_alpha = sinf(alpha);
		cos_alpha = cosf(alpha);
		sin_beta = sinf(beta);
		cos_beta = cosf(beta);
    }
    const F32 SinAlpha() const { return sin_alpha; }
    const F32 CosAlpha() const { return cos_alpha; }
    const F32 SinBeta()  const { return sin_beta; }
    const F32 CosBeta()  const { return cos_beta; }
};

class SinAlphaBetaGammaDelta
{
    private:
    union
    {
        struct { F32 sin_alpha, sin_beta, sin_gamma, sin_delta; };
        //struct { U128 packed; };
    };
    
    public:
    SinAlphaBetaGammaDelta( F32 alpha, F32 beta, F32 gamma, F32 delta )
    {
        //packed = fast_sin_alpha_beta_gamma_delta( alpha, beta, gamma, delta );
		sin_alpha = sinf(alpha);
		sin_beta = sinf(beta);
		sin_gamma = sinf(gamma);
		sin_delta = sinf(delta);
    }
    const F32 SinAlpha() const { return sin_alpha; }
    const F32 SinBeta()  const { return sin_beta; }
    const F32 SinGamma() const { return sin_gamma; }
    const F32 SinDelta() const { return sin_delta; }
};

class CosAlphaBetaGammaDelta
{
    private:
    union
    {
        struct { F32 cos_alpha, cos_beta, cos_gamma, cos_delta; };
        //struct { U128 packed; };
    };
    
    public:
    CosAlphaBetaGammaDelta( F32 alpha, F32 beta, F32 gamma, F32 delta )
    {
        //packed = fast_cos_alpha_beta_gamma_delta( alpha, beta, gamma, delta );
		cos_alpha = cosf(alpha);
		cos_beta = cosf(beta);
		cos_gamma = cosf(gamma);
		cos_delta = cosf(delta);
    }
    const F32 CosAlpha() const { return cos_alpha; }
    const F32 CosBeta()  const { return cos_beta; }
    const F32 CosGamma() const { return cos_gamma; }
    const F32 CosDelta() const { return cos_delta; }
};


