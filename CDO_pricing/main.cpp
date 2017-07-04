#define _USE_MATH_DEFINES

#include "SteinPricer.h"
#include "GaussianPricer.h"
#include "NIGPricer.h"
#include "zero_search.h"
#include "QMCGenerators.h"


using namespace MCPROJ;

//This is the main

/*
template <typename Gen>
class percent_default_vr {
public:
	percent_default_vr(Gen X, double K1, double K2, std::vector<double> theta) :X(X), K1(K1), K2(K2), theta(theta) {};
	~percent_default_vr() {};
	double operator()() {
		double C = normal_CDF_inverse(q);
		double counter = 0;
		double x;

		double M = X();
		double theta_prod_X = theta[0]*M;
		double scalar_pdt_theta = theta[0] * theta[0];

		//if(Gen.name== normal)
		
		for (int i = 0; i < Nb_CDS; i++) {
			x = X();
			if (x+theta[i+1] < (C - corr*(M+theta[0])) / sqrt(1 - corr*corr)) { counter ++; }
			theta_prod_X += x*theta[1+i];
			scalar_pdt_theta += theta[1+i] * theta[1+i];
		}

		counter *= exp(-theta_prod_X-(scalar_pdt_theta / 2));
		counter = std::max((counter*(1 - R) / Nb_CDS) - K1, 0.0) / (K2 - K1);			//Normalization of counter in K1 and K2
		return counter;
	}
private:
	Gen X;
	double K1, K2;
	std::vector<double> theta;
};




template <typename Gen>
std::vector<double> grad_f(int i, std::vector<double> theta, std::vector<std::vector<double>> stored,
							int Nb_CDS, Gen X, double q, double corr, double K1, double K2) {//ALL these will be attributes 
	std::vector<double> res(Nb_CDS + 1, 0);
	double C = normal_CDF_inverse(q);
	for (int j = 0; j < Nb_CDS + 1; j++) {
		for (int k = 0; k < 1000; k++) {
			int counter = 0;
			double scalar_product = theta[0] * stored[k][0];
			double square_theta = theta[0] * theta[0];
			for (int r = 1; r < Nb_CDS+1; r++) {
				if (stored[k][r] < (C - corr*stored[k][0]) / sqrt(1 - corr*corr)) { counter++; }
				scalar_product += theta[r] * stored[k][r];
				square_theta += theta[r] * theta[r];
			}
			double A = std::max((counter*(1 - R) / Nb_CDS) - K1, 0.0) / (K2 - K1);
			A *= exp(-scalar_product + square_theta / 2);
			res[j] += A*((theta[j] - stored[k][j])*(theta[i] - stored[k][i]) + (i == j ? 1 : 0));
			res[j] /= 1000;
		}
	}
	return res;
}

template <typename Gen>
std::vector<double> test(std::vector<double> theta, std::vector<std::vector<double>> stored,
	int Nb_CDS, Gen X, double q, double corr, double K1, double K2) {//ALL these will be attributes 
	std::vector<double> res(Nb_CDS + 1, 0);
	double C = normal_CDF_inverse(q);
	for (int j = 0; j < Nb_CDS + 1; j++) {
		for (int k = 0; k < 1000; k++) {
			int counter = 0;
			double scalar_product = theta[0] * stored[k][0];
			double square_theta = theta[0] * theta[0];
			for (int r = 1; r < Nb_CDS + 1; r++) {
				if (stored[k][r] < (C - corr*stored[k][0]) / sqrt(1 - corr*corr)) { counter++; }
				scalar_product += theta[r] * stored[k][r];
				square_theta += theta[r] * theta[r];
			}
			double A = std::max((counter*(1 - R) / Nb_CDS) - K1, 0.0) / (K2 - K1);
			A *= exp(-scalar_product + square_theta / 2);
			res[j] += A*(theta[j] - stored[k][j]);
			res[j] /= 1000;
		}
	}
	return res;
}

template <typename Gen>
std::vector<double> optimizer_Theta(int Nb_CDS, Gen X, double q, double corr, double K1, double K2) //ALL these will be attributes
{


	double C = normal_CDF_inverse(q);
	std::vector<std::vector<double>> stored (1000, std::vector<double>(Nb_CDS+1));
	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < Nb_CDS + 1; j++)
		{
			stored[i][j] = X();
		}
	}



	//algorithm
	std::vector<double> theta0(Nb_CDS + 1, 0);

	generator gen;
	boost::random::uniform_int_distribution<> dist(0, 100);

	double step = 1e-1;

	double t = 0;
	std::vector<double> eps(Nb_CDS + 1, 1e-5);
	while (test(theta0, stored, Nb_CDS, X, q, corr, K1, K2)> eps) { //ALL these will be attributes ) > eps) {
		t += 1;
		std::cout << "iteration " << t << std::endl;
		int i = dist(gen);
		for(int y=0; y < Nb_CDS + 1; y++){
		theta0[y] -= step / (sqrt(t)) * grad_f(i, theta0, stored,
											Nb_CDS, X, q, corr, K1, K2)[y]; //ALL these will be attributes );
		}
	} 

	return theta0;

};
*/

int main() {
	
	double q = 0.5;   //default probability
	double corr = 0.3;	//correlation between CDS
	double R = 0;		//recovery rate
	int		Nb_CDS = 100;
	double	K1 = 0.2;
	double	K2 = 0.7;
	double alpha = 1;
	double beta = 0.5;

	int gridSize = 1000;

	/*

	PoissonSteinPricer PoissonStein(std::string("Gaussian"), q, corr, R, Nb_CDS, K1, K2);

	std::vector<double> vect9 = PoissonStein.price(10000);

	std::cout << vect9[0] << "    " << vect9[1] << "    " << vect9[2] << std::endl;

	PoissonSteinPricer PoissonStein2(std::string("NIG"), q, corr, R, Nb_CDS, K1, K2, alpha, beta, gridSize);

	std::vector<double> vect10 = PoissonStein2.price(100);

	std::cout << vect10[0] << "    " << vect10[1] << "    " << vect10[2] << std::endl;
	
	GaussianSteinPricer GaussStein(std::string("Gaussian"), q, corr, R, Nb_CDS, K1, K2);

	std::vector<double> vect3 = GaussStein.price(10000);

	std::cout << vect3[0] << "    " << vect3[1] << "    " << vect3[2] << std::endl;
	
	GaussianSteinPricer GaussStein2(std::string("NIG"), q, corr, R, Nb_CDS, K1, K2, alpha, beta, gridSize);

	std::vector<double> vect4 = GaussStein2.price(1000);

	std::cout << vect4[0] << "    " << vect4[1] << "    " << vect4[2] << std::endl;
	
	
	*/

	GaussianPricer Gauss(q, corr, R, Nb_CDS, K1, K2);

	std::cout << Gauss.expected_Loss() << std::endl ;

	std::vector<double> vect = Gauss.expected_LossMC(10000);
	std::cout << vect[0] << "    " << vect[1] << "    " << vect[2] << std::endl;

	double rofl = Gauss.expected_LossQMC(100, "Halton");
	std::cout << rofl << std::endl;

	double rofl1 = Gauss.expected_LossQMC(100, "Kakutani");
	std::cout << rofl1 << std::endl;

	

	NIGPricer NIG(q, corr, R, Nb_CDS, K1, K2, alpha, beta, gridSize);

	std::vector<double> vect2 = NIG.expected_LossMC(100);
	std::cout << vect2[0] << "    " << vect2[1] << "    " << vect2[2] << std::endl;

	
	double vect_667 = NIG.expected_LossQMC(100, "Halton");
	std::cout << vect_667 << std::endl;

	double vect_668 = NIG.expected_LossQMC(100, "Kakutani");
	std::cout << vect_668 << std::endl;

	/**************************************/
	/*
	Kakutani2D KGen(2, 3, 10);
	for (int i = 0; i < 30; i++) {
		std::vector<double> lol = KGen();
		std::cout<<lol[0]<< " \t " <<lol[1]<<std::endl;
	}

	std::vector<int> p = KGen.double2piadic(1 / 5.0 + 3 / 25.0, 5);
	for (int i = 0; i < p.size(); i++) {
		std::cout << p[i] << std::endl;
	}

	double a = KGen.padic2double(p, 5);
	std::cout << a << std::endl;*/
	int lol;
	std::cin >> lol;

	return 0;

	/*generator gen;
	normal_rv G(gen, normal_dist(0, 1));
	gen.seed(static_cast<unsigned int>(std::time(0)));
	percent_default<normal_rv> pd1(G, 0.2, 0.7);                        //percentage default generator with normal random

	std::vector<double>theta = optimizer_Theta(Nb_CDS, G, q, corr, 0.2, 0.7);
	
	percent_default_vr<normal_rv> pd2(G, 0.2, 0.7, theta);   //percentage default generator with normal random

	
	std::vector<double> result = monte_carlo(1000, pd1);

	std::cout << "without vr" << result[0] <<'\t'<< result[1] << '\t' << result[2] << std::endl;

	result = monte_carlo(1000, pd2);
	std::cout << "vr" <<result[0] << '\t' << result[1] << '\t' << result[2] << std::endl;
	
	std::cout << expected_loss(0.2, 0.7) << std::endl;
	*/


}