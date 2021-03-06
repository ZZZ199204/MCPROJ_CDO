#include "GaussianPricer.h"


MCPROJ::GaussianPricer::GaussianPricer(double q, double corr, double R, int	Nb_CDS, double K1, double K2) 
	: m_q(q), m_corr(corr), m_R(R), m_Nb_CDS(Nb_CDS), m_K1(K1), m_K2(K2){
	

	m_G = new normal_rv(m_gen, normal_dist(0, 1));
	m_gen.seed(static_cast<unsigned int>(0));//to remove later
	m_HaltonGauss = new Halton2DGauss();
	m_KakutaniGauss = new Kakutani2DGauss();

	m_C = normal_CDF_inverse(q);


};


double MCPROJ::GaussianPricer::expected_Loss() {

	if (m_R == 1 || m_K1 / (1 - m_R) > 1 || m_K2 / (1 - m_R) > 1) { throw std::exception("Unreasonable K and R parameters: R=1 or K/(1-R)>1"); }
	else {
		double K11 = normal_CDF_inverse(m_K1 / (1 - m_R));
		double K21 = normal_CDF_inverse(m_K2 / (1 - m_R));
		return (BivariateNormalCDF(-K11, m_C, -sqrt(1 - m_corr*m_corr)) - BivariateNormalCDF(-K21, m_C, -sqrt(1 - m_corr*m_corr))) * (1 - m_R) / (m_K2 - m_K1);
	}

};


double MCPROJ::GaussianPricer::operator()() {

	return MCPROJ::percentage_default(*m_G, *m_G, m_C, m_corr, m_R, m_Nb_CDS, m_K1, m_K2);

};

std::vector<double> MCPROJ::GaussianPricer::expected_LossMC(int N) {

	MCPROJ::GaussianPricer gauss(m_q, m_corr, m_R, m_Nb_CDS, m_K1, m_K2);

	return MCPROJ::monte_carlo(N, gauss);

}

double MCPROJ::GaussianPricer::derivativeOfVarTranches(int N, double theta) {
	double x;
	double res = 0;
	for (int j = 0; j < N; j++) {
		x = importance_sampling_tranches(theta);
		res += x;
	}
	res /= (double)N;
	
	return res;

};

double MCPROJ::GaussianPricer::importance_sampling_tranches(double theta)
{
	double counter = 0;
	double x;
	double sum = 0;

	double Mvalue = m_G->operator()();

	for (int i = 0; i < m_Nb_CDS; i++) {
		x = m_G->operator()();
		sum += x;
		if (x < (m_C - m_corr*Mvalue) / sqrt(1 - m_corr*m_corr)) { counter++; }
	}
	counter = std::min(std::max((counter*(1 - m_R) / m_Nb_CDS) - m_K1, 0.0), m_K2 - m_K1) / (m_K2 - m_K1);		//Normalization of counter in K1 and K2
	
	return (m_Nb_CDS * theta - sum) * counter * counter * exp(-theta * sum + m_Nb_CDS * theta * theta / 2);
	
};

double MCPROJ::GaussianPricer::derivativeOfVarCommon(int N, double theta) {
	double x;
	double res = 0;
	for (int j = 0; j < N; j++) {
		x = importance_sampling_common(theta);
		res += x;
	}
	res /= (double)N;

	return res;

};

double MCPROJ::GaussianPricer::importance_sampling_common(double theta)
{
	double counter = 0;
	double x;
	double sum = 0;

	double Mvalue = m_G -> operator()();

	for (int i = 0; i < m_Nb_CDS; i++) {
		x = m_G -> operator()();
		if (x < (m_C - m_corr*Mvalue) / sqrt(1 - m_corr*m_corr)) { counter++; }
	}
	counter = std::min(std::max((counter*(1 - m_R) / m_Nb_CDS) - m_K1, 0.0), m_K2 - m_K1) / (m_K2 - m_K1);		//Normalization of counter in K1 and K2

	return (theta - Mvalue) * counter * counter * exp(-theta * Mvalue  +  theta * theta / 2);

}
std::vector<double> MCPROJ::GaussianPricer::expected_LossMCVR(int N, double thetaCommon, double thetaTranches)
{
	std::vector<double> result(3, 0.0);
	double x;
	for (int j = 0; j < N; j++) {
		x = MCPROJ::percentage_defaultVR(*m_G, *m_G, m_C, m_corr, m_R, m_Nb_CDS, m_K1, m_K2, thetaCommon, thetaTranches);
		x *= exp((-thetaCommon * thetaCommon - m_Nb_CDS * thetaTranches * thetaTranches) / 2);
		result[0] += x;
		result[1] += x*x;
	}
	result[0] /= (double)N;
	result[1] = (result[1] - N*result[0] * result[0]) / (double)(N - 1);
	result[2] = 1.96*sqrt(result[1] / (double)N);
	return result;

};



double MCPROJ::GaussianPricer::expected_LossQMC(int N, std::string Qtype)
{
	double result = 0.0;
	double x;

	if(Qtype=="Halton")
		for (int j = 0; j < N; j++) {
			x = percentage_defaultQMCH(m_C, m_corr, m_R, m_Nb_CDS, m_K1, m_K2); // restriction sur X...
			result += x;
		}

	else if (Qtype == "Kakutani")
		for (int j = 0; j < N; j++) {
			x = percentage_defaultQMCK(m_C, m_corr, m_R, m_Nb_CDS, m_K1, m_K2); // restriction sur X...
			result += x;
		}
	else { throw std::exception(" Only Halton and Kakutani are accepted as input string "); }


	result /= (double)N;
	return result;
};

double MCPROJ::GaussianPricer::percentage_defaultQMCH(double C, double corr, double R, int Nb_CDS, double K1, double	K2) {
	double counter = 0;
	double x;

	double Mvalue = m_HaltonGauss->operator()()[0];

	for (int i = 0; i < Nb_CDS; i++) {
		x = m_HaltonGauss->operator()()[1];
		if (x < (C - corr*Mvalue) / sqrt(1 - corr*corr)) { counter++; }
	}
	counter = std::min(std::max((counter*(1 - R) / Nb_CDS) - K1, 0.0), K2 - K1) / (K2 - K1);		//Normalization of counter in K1 and K2
																									//counter *= (1 - R) / Nb_CDS;
	return counter;
};

double MCPROJ::GaussianPricer::percentage_defaultQMCK(double C, double corr, double R, int Nb_CDS, double K1, double	K2) {
	double counter = 0;
	double x;

	double Mvalue = m_KakutaniGauss->operator()()[0];

	for (int i = 0; i < Nb_CDS; i++) {
		x = m_KakutaniGauss->operator()()[1];
		if (x < (C - corr*Mvalue) / sqrt(1 - corr*corr)) { counter++; }
	}
	counter = std::min(std::max((counter*(1 - R) / Nb_CDS) - K1, 0.0), K2 - K1) / (K2 - K1);		//Normalization of counter in K1 and K2
																									//counter *= (1 - R) / Nb_CDS;
	return counter;
};

