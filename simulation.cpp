#include <bits/stdc++.h> 

using namespace std;

struct user{
	double entry_time;
	double transfer;
	int currency;
	double priv_parameter;
};

struct pool{
	double size;
	int currency;
	int label;
};

struct LP{
	double entry_time;
	double size_lp;
	double opport_cost;
	int pool_label; 
	int currency;
};

double cost(double size, double transfer){
	return transfer / size;
}

double type_var, transfer_var, opportunity_var, sizeLP_var;
int NMax, Nlp, months, month_length;
int i,j;
const int Npools = 1000;
const int Ncurrencies = 100;
double c[Ncurrencies][Ncurrencies], cLP[Npools][Npools];
int cur_cap[Ncurrencies];
double poolLeaving[Npools], poolEntering[Npools];
double pool_sizes[Npools];
pool pools[Npools];
int n_pools, n_currencies;
double averageArrival; 
double lambda;
vector <LP> LP_sets[Npools];
int pools_initial[Npools]; 

int usr_best_pool(user usr){
	return 0;
}

int LP_best_pool(LP nLP){
	return 0;
}


void pool_currency_init(){//initialize number of pools and pool currencies
	n_pools	= 5;
	pools[0].currency = 0;
	pools[1].currency = 0;
	pools[2].currency = 1;
	pools[3].currency = 1;
	pools[4].currency = 2;
	return;
}

void pool_sizes_init(){//initialize pool sizes 
	pools[0].size = 10000000;
	pools[1].size = 1000000;
	pools[2].size = 1000000;
	pools[3].size = 500000;
	pools[4].size = 300000;	
	return;
}

void pool_initial_members(){//initialize the decomposition of initial pools into LP's 
	for (i = 0; i < n_pools; i++){
		pools_initial[i] = 10;
	}
	for (i = 0; i < n_pools; i++){
		for (j = 0; j < pools_initial[i]; j++){
			LP cur;
			cur.entry_time = 0;
			cur.size_lp = pools[i].size / pools_initial[i];
			cur.opport_cost = 0.05;
			cur.pool_label = i;	 
			cur.currency = pools[i].currency;
			LP_sets[i].push_back(cur);
		}
	}
	return;	
}

void initialization(){//initialize pools and parameters
	pool_currency_init();
	
	pool_sizes_init();

	for (i = 0; i < n_pools; i++){
		pools[i].label = i;
	}

	pool_initial_members();

	NMax = 1000000;
	Nlp  = 1000;

	n_currencies = pools[n_pools - 1].currency + 1;
	cur_cap[0] = 500000;
	cur_cap[1] = 800000;
	cur_cap[2] = NMax;

	for (i = 0; i < n_currencies; i++){
		for (j = 0; j < n_currencies; j++){
			if (i != j){
				c[i][j] = 1.05;
			}
		}
	}

	for (i = 0; i < n_pools; i++)
		poolLeaving[i] = 1.05;

	for (i = 0; i < n_pools; i++)
		poolEntering[i] = 1.05;

	for (i = 0; i < n_pools; i++){
		for (j = 0; j < n_pools; j++){
			if (i != j){
				cLP[i][j] = poolLeaving[i] * c[pools[i].currency][pools[j].currency] * poolEntering[j];
			}
		}
	}
	
	type_var = 1.0;
	transfer_var = 1000;
	opportunity_var = 0.1;
	sizeLP_var = 10.0;

	averageArrival = 1.5;
	lambda = 1 / averageArrival;
	
	return;
}

int main (){
	initialization();

	// seed the RNG
	random_device rd; // uniformly-distributed integer random number generator
	mt19937 rng (rd ()); // mt19937: Pseudo-random number generation

	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator (seed);

	normal_distribution <double> distr_type (0.0, type_var);
	normal_distribution <double> distr_transfer(0.0, transfer_var);
	normal_distribution <double> distr_opportunity(0.0, opportunity_var);
	normal_distribution <double> distr_sizeLP(0.0, sizeLP_var);	

	cout << "some Normal-distributed(0.0,1.0) results:" << std::endl;

	exponential_distribution<double> exp (lambda);
	double sumArrivalTimes=0;
	double newArrivalTime;
	months = 1;
	for (int i = 0; i < 10; i++){
		newArrivalTime=  exp.operator() (rng);// generates the next random number in the distribution 
		sumArrivalTimes  = sumArrivalTimes + newArrivalTime;  
		cout << sumArrivalTimes << endl;
		int gen = rand() % NMax;
		if (gen < Nlp){//here we generate an LP
			LP *nLP = new LP();
			nLP->entry_time = sumArrivalTimes;
			nLP->size_lp = fabs(distr_sizeLP(generator));
			nLP->opport_cost = fabs(distr_opportunity(generator));
			int cur_gen = rand() % NMax;
			for (i = 0; i < Ncurrencies; i++)
				if (cur_cap[i] > cur_gen){
					nLP->currency = i;
					break;
				}
			int ind_pool = LP_best_pool(*nLP);
			//add nLP to the pool indexed ind_pool and update it
			
		} else{//here we generate user
			user *usr = new user();
			usr->entry_time = sumArrivalTimes;
			usr->transfer = fabs(distr_transfer(generator));
			usr->priv_parameter = fabs(distr_type(generator));
			int cur_gen = rand() % NMax;
			for (i = 0; i < Ncurrencies; i++)
				if (cur_cap[i] > cur_gen){
					usr->currency = i;
					break;
				}
			int ind_pool = usr_best_pool(*usr);
			//in the pool ind_pool update all LP's incomes
					
		}
		if (sumArrivalTimes > months * month_length){//we check for all LP's if they want to leave and update pools
			months++;		
		}
	}
	return 0;
}
