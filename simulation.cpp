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
	double avg_gains;
};

struct LP{
	double entry_time;
	double size_lp;
	double opport_cost;
	double gains;
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
	int i, ans;
	double min_loss = 1000.0;
	for (i = 0; i < n_pools; i++){
		double cur_loss = 1.0;
		if (usr.currency != pools[i].currency) cur_loss *= c[usr.currency][pools[i].currency];
		cur_loss *= (1.0 + cost(pools[i].size, usr.transfer));
		if (min_loss > cur_loss){
			ans = i;
			min_loss = cur_loss; 	
		}
	}
	return ans;
}

int LP_best_pool(LP nLP){
	int i, ans;
	double min_loss = 1000.0;
	for (i = 0; i < n_pools; i++){
		double cur_loss = 1.0;
		if (nLP.currency != pools[i].currency) cur_loss *= c[nLP.currency][pools[i].currency];
		cur_loss *= (1.0 + pools[i].avg_gains);
		if (min_loss > cur_loss){
			ans = i;
			min_loss = cur_loss; 	
		}
	}
	return ans;
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

void init_trans_coefficients(){
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
	return;
}

void init_meta_parameters(){
	type_var = 1.0;
	transfer_var = 1000;
	opportunity_var = 0.1;
	sizeLP_var = 10.0;

	averageArrival = 1.5;
	lambda = 1 / averageArrival;
	return;
}

void initialization(){//initialize pools and parameters
	pool_currency_init();
	
	pool_sizes_init();

	for (i = 0; i < n_pools; i++){
		pools[i].label = i;
		pools[i].avg_gains = 1.0;
	}

	pool_initial_members();

	NMax = 1000000;
	Nlp  = 1000;

	init_trans_coefficients();
	init_meta_parameters();

	n_currencies = pools[n_pools - 1].currency + 1;

	cur_cap[0] = 500000;
	cur_cap[1] = 800000;
	cur_cap[2] = NMax;
	
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
	for (int i = 0; i < 1000000; i++){
		newArrivalTime=  exp.operator() (rng);// generates the next random number in the distribution 
		sumArrivalTimes  = sumArrivalTimes + newArrivalTime;  
		//cout << sumArrivalTimes << endl;
		int gen = rand() % NMax;
		if (gen < Nlp){//here we generate an LP
			LP *nLP = new LP();
			nLP->entry_time = sumArrivalTimes;
			nLP->size_lp = fabs(distr_sizeLP(generator));
			nLP->opport_cost = fabs(distr_opportunity(generator));
			nLP->gains = 0;
			int cur_gen = rand() % NMax;
			for (i = 0; i < Ncurrencies; i++)
				if (cur_cap[i] > cur_gen){
					nLP->currency = i;
					break;
				}
			int ind_pool = LP_best_pool(*nLP);
			//add nLP to the pool indexed ind_pool and update it
			LP_sets[ind_pool].push_back(*nLP);
			pools[ind_pool].size += nLP->size_lp;
			nLP->pool_label = ind_pool;
			
		} else{//here we generate user
			user *usr = new user();
			usr->entry_time = sumArrivalTimes;
			usr->transfer = fabs(distr_transfer(generator));
			usr->priv_parameter = fabs(distr_type(generator));
			int cur_gen = rand() % NMax;
			for (int j = 0; j < Ncurrencies; j++){//here we generate currency
				if (cur_cap[j] > cur_gen){
					usr->currency = j;
					break;
				}
			}
			int ind_pool = usr_best_pool(*usr);
			//in the pool ind_pool update all LP's incomes
			for (int j = 0; j < LP_sets[ind_pool].size(); j++){
				//we assume proportional distribution of gains, some other function can also be considered
				LP_sets[ind_pool][j].gains += usr->transfer * cost(pools[ind_pool].size, LP_sets[ind_pool][j].size_lp) * LP_sets[ind_pool][j].size_lp / pools[ind_pool].size; 
			}
					
		}
		if (sumArrivalTimes > months * month_length){//we check for all LP's if they want to leave and update pools, also average monthly gains
			months++;
			for (int i = 0; i < n_pools; i++){
				double avg_monthly_gains = 1.0;
				int activeLPs = 0;
				for (int j = 0; j < n_pools; j++){
					if (LP_sets[i][j].pool_label >= 0){
						LP cur = LP_sets[i][j];
						if (cur.gains < cur.size_lp * cur.opport_cost){//LP wants to leave and pool is resized
							pools[cur.pool_label].size -= cur.size_lp;
							cur.pool_label = -1;
						}
						avg_monthly_gains *= (1.0 + cur.gains);
						activeLPs++;
						cur.gains = 0;
						LP_sets[i][j] = cur;
					}
				}
				pools[i].avg_gains = pow(avg_monthly_gains, 1.0 / activeLPs);
				cout << pools[i].size << " " << pools[i].avg_gains << "     ";
			}		
			cout << "\n";
		}
	}
	return 0;
}
