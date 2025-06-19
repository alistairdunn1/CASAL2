/**
 * @file ProportionsCategoryByLength.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/05/02
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

// Headers
#include "ProportionsCategoryByLength.h"

#include <iomanip>
#include <iostream>

#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Observations/Manager.h"
#include "TestResources/Models/TwoSexLengthComplex.h"
#include "TestResources/TestFixtures/InternalEmptyLengthModel.h"

// Namespaces
namespace niwa {
namespace length {
using niwa::testfixtures::InternalEmptyLengthModel;
using std::cout;
using std::endl;

/**
 * This unit test is specifically for ensuring we have the right values
 * being populated in the comparison object. Very carefully we're concerned
 * about the selectivities and the categories they're associated with.
 */
TEST_F(InternalEmptyLengthModel, Observation_Length_ProportionsCategoryByLength_TwoSexComplex) {
  const std::string observation_definition =
      R"(
        @observation observation
        type proportions_by_category        
        time_step step_one
        categories immature.male+immature.female immature.female
        selectivities logistic_one logistic_two logistic_three
        total_categories mature.male immature.female+mature.male
        selectivities_for_total_categories logistic_four logistic_five logistic_six
        likelihood binomial
        time_step_proportion 1.0
        length_bins 5:9
        delta 1e-5
        plus_group true
        years 2007 2008
        table obs
        2007 0.01 0.02 0.03 0.04 0.05 0.06 0.07 0.08 0.09 0.1
        2008 0.02 0.03 0.04 0.05 0.06 0.07 0.08 0.09 0.1 0.11
        end_table
        table error_values
        2007 0.1 
        2008 0.1 0.11 0.12 0.13 0.14 0.15 0.16 0.17 0.18 0.19
        end_table
    )";

  AddConfigurationLine(testresources::models::two_sex_length_complex, "TwoSexLengthComplex.h", 23);
  AddConfigurationLine(observation_definition, __FILE__, 37);
  LoadConfiguration();

  model_->Start(RunMode::kTesting);
  model_->FullIteration();

  auto observation_ptr = model_->managers()->observation()->GetObservation("observation");
  ASSERT_NE(nullptr, observation_ptr);

  const vector<obs::Comparison>& comparisons = observation_ptr->comparisons(2008);
  ASSERT_EQ(10u, comparisons.size());

  // Check first comparison
  unsigned index = 0;
  EXPECT_EQ("immature.male+immature.female", comparisons[index].category_);
  ASSERT_EQ(3u, comparisons[index].selectivities_.size());
  auto sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_four", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_one", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_two", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(5.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(1.7758874054547531, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(0.02, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(0.1, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.1, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(1e-5, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(2.2302192602449185, comparisons[index].score_);

  // Check second comparison
  index = 7;
  EXPECT_EQ("immature.female", comparisons[index].category_);
  ASSERT_EQ(3u, comparisons[index].selectivities_.size());
  sel_it = comparisons[index].selectivities_.begin();
  EXPECT_EQ("logistic_five", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_six", *sel_it);
  ++sel_it;
  EXPECT_EQ("logistic_three", *sel_it);
  EXPECT_EQ(0u, comparisons[index].age_);
  EXPECT_DOUBLE_EQ(7.0, comparisons[index].length_);
  EXPECT_DOUBLE_EQ(0.0090571759486253213, comparisons[index].expected_);
  EXPECT_DOUBLE_EQ(0.09, comparisons[index].observed_);
  EXPECT_DOUBLE_EQ(0.0, comparisons[index].process_error_);
  EXPECT_DOUBLE_EQ(0.17, comparisons[index].error_value_);
  EXPECT_DOUBLE_EQ(0.17, comparisons[index].adjusted_error_);
  EXPECT_DOUBLE_EQ(1e-5, comparisons[index].delta_);
  EXPECT_DOUBLE_EQ(0.06991056784978654, comparisons[index].score_);
}

const std::string simple_single_sex_model =
    R"(
@model
type length
start_year 1986 
final_year 2012
projection_final_year 2021
length_bins  1:68
length_plus t
length_plus_group 69
base_weight_units tonnes
initialisation_phases Equilibrium_state
time_steps Annual

@categories 
format tag
names untagged tag_1996
growth_increment growth_model growth_model

@initialisation_phase Equilibrium_state
type iterative
years 200
convergence_years 200

@time_step Annual 
processes Recruit_BH growth  mortality tag_1996

@process Recruit_BH
type recruitment_beverton_holt
ssb_offset 1
standardise_years 1986:2010
recruitment_multipliers 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00  1.00 1.00 1.00  1.00 1.00 1.00
b0 1500
categories untagged
proportions 1
steepness 0.75
initial_mean_length 10
initial_length_cv 0.40
ssb SSB

@process tag_1996
type tagging
years 1996
from untagged
to tag_1996
initial_mortality 0
u_max 0.9
selectivities [type=constant; c=1]
penalty none
N 40
table proportions
year 1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32	33	34	35	36	37	38	39	40	41	42	43	44	45	46	47	48	49	50	51	52	53	54	55	56	57	58	59	60	61	62	63	64	65	66	67	68
1996 0	0	0	0	0	0	0	0		0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0.025	0	0.05	0.025	0	0	0.025	0.05	0	0.05	0.175	0.15	0.025	0.025	0.075	0.05	0.025	0.075	0.05	0.025	0.025	0.05	0	0.025	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0 0 
end_table

@process mortality
type mortality_instantaneous
m 0.2 * 2
relative_m_by_length One * 2
time_step_proportions 1
categories *
table catches
year Trawl_1
1987	0
1988	0
1989	0
1990	16.19729814
1991	136.5431735
1992	64.99411365
1993	73.01365139
1994	29.60436938
1995	66.48247047
1996	107.1336092
1997	60.94452902
1998	150.8267401
1999	126.7738246
2000	101.141994
2001	28.29833853
2002	65.25326787
2003	77.43894769
2004	39.37986312
2005	33.22696512
2006	35.71771887
2007	35.93582888
2008	55.00361249
2009	31.55442523
2010	85.05248259
2011	81.7369449
2012	60.51004034
end_table
table method
method        category  selectivity 	 u_max  time_step  penalty
Trawl_1  untagged  		FishingSel_1     0.9   	Annual    	none
Trawl_1  tag_1996  		FishingSel_1     0.9   	Annual    	none
end_table

@process growth
type growth
categories untagged tag_1996

@growth_increment growth_model
type basic
l_alpha 20
l_beta  40
g_alpha 10
g_beta 1
min_sigma 2
distribution normal
length_weight allometric
cv 0.0
compatibility_option casal

@length_weight allometric
type basic
a 0.000000000373
b 3.145
units tonnes

@derived_quantity SSB
type biomass
categories untagged tag_1996
selectivities maturity * 2
time_step Annual
time_step_proportion 0.5

@selectivity double_normal
type double_normal
mu 21
sigma_l 5
sigma_r 10

@selectivity maturity
type logistic
a50 30 
ato95 5

@selectivity FishingSel_1
type logistic
a50 24 
ato95 5

@selectivity One
type constant
c 1

@selectivity TrawlSurveySel_1
type logistic 
a50 30 
ato95 3.5
)";

const std::string bespoke_length_range =
    R"(
@observation TrawlSurveyProportionAtLength
type proportions_by_category
years 1993 1994 1995 1998 2000 2001 2002 2008 2012 
likelihood binomial
time_step Annual
categories untagged
total_categories untagged
time_step_proportion 0.5
selectivities TrawlSurveySel_1
selectivities_for_total_categories One
length_bins 35:68 
plus_group true
delta 1e-5
table obs
1993	0.0140919	0.012673	0.0164306	0.0145034	0.0145935	0.0194405	0.0213371	0.0262892	0.0267303	0.0269179	0.0282809	0.031152	0.0381955	0.035735	0.0271571	0.0296743	0.017894	0.018616	0.00694274	0.0053634	0.00298689	0.0010565	0.000815978	0.000161981	0.000277566	0	0	0	0	0	0	0	0	0
1994	0.0182902	0.022186	0.0253445	0.0206195	0.0241757	0.0275888	0.0262872	0.0321282	0.028852	0.036786	0.0353481	0.0321946	0.0290965	0.0309325	0.0243934	0.0219202	0.0142894	0.0131678	0.00855243	0.00598131	0.00258991	0.00183351	0.00165922	0.000585514	0.000465192	0.000262052	6.16E-05	0	0	0	0	0	0	0
1995	0.0174651	0.0178496	0.0184049	0.0269278	0.0279746	0.0328895	0.0292305	0.0339443	0.0323902	0.0415441	0.0450913	0.046253	0.0402894	0.0387133	0.0312197	0.0259501	0.0158408	0.0145074	0.00889533	0.00588692	0.00343534	0.00165713	0.00131754	0.000677305	0.000468389	0.000108561	7.34E-05	0.000207115	0	0	0	0	0	0
1998	0.00997162	0.0141141	0.0145014	0.0126108	0.0208807	0.0181872	0.0233474	0.0273881	0.0255188	0.0355096	0.0347917	0.041853	0.0393915	0.0492512	0.0376406	0.0406878	0.0371633	0.0311177	0.0303821	0.0163014	0.0123806	0.0122209	0.00625397	0.00219066	0.00119053	0.000684215	0.000269435	0	0	0	0	0	0	0
2000	0.00596328	0.0130699	0.0151415	0.0166723	0.0117753	0.0150501	0.0183889	0.0183243	0.0251793	0.0272067	0.0302888	0.0426445	0.0344711	0.0459327	0.0546313	0.0280818	0.0233143	0.0170249	0.0120725	0.00637217	0.00672323	0.00605017	0.00237238	0.00195698	0.00361508	0.000739747	0.000369874	0	0	0	0	0	0	0
2001	0.00890307	0.00779348	0.0106324	0.0143166	0.0139099	0.0165094	0.00969166	0.0289338	0.0320583	0.0265763	0.0439684	0.0519121	0.062867	0.0619169	0.030755	0.02868	0.0219375	0.0186048	0.00955954	0.00879438	0.00578588	0.00437727	0.00213456	0.00079413	0.000948447	0	0	0	0	0	0	0	0	0
2002	0.014472	0.0188418	0.0259168	0.017065	0.0165564	0.0206997	0.0188988	0.0227266	0.0282959	0.028683	0.0255729	0.0351423	0.0317747	0.0231001	0.0273721	0.0175139	0.0125888	0.00929346	0.00618996	0.00365564	0.00345773	0.00110963	0	0	0	0.000554815	0	0	0	0	0	0	0	0
2008	0.0282501	0.0230282	0.0235084	0.0273027	0.0291135	0.028831	0.0319954	0.0224326	0.0207084	0.0224083	0.0185736	0.0193064	0.0238063	0.0313935	0.0286175	0.0248732	0.0241705	0.0154054	0.0133991	0.00810026	0.00371131	0.000962458	0.000624837	0.000969987	0.000202421	0	0	0.000212628	0	0	0	0	0	0
2012	0.0134243	0.0122425	0.0145892	0.0176558	0.0174679	0.0200321	0.0191565	0.0194462	0.0187401	0.0180736	0.0226931	0.0158082	0.0180017	0.0187405	0.0259654	0.0275044	0.0214882	0.0178625	0.0143406	0.0121681	0.00864093	0.00747305	0.0035931	0.0033777	0.00148888	0	0.000620655	0.000400628	0.00027109	0	0	0	0	0
end_table
table error_values
1993 49.0
1994 46.3
1995 49.2
1998 41.7
2000 43.3
2001 40.3
2002 37.4
2008 39.7
2012 36.8
end_table
)";

const std::string default_length_range =
    R"(
@observation TrawlSurveyProportionAtLength
type proportions_by_category
years 1995 1998 2000 2001 2002 2008 2012 
likelihood binomial
time_step Annual
categories untagged
total_categories untagged
time_step_proportion 0.5
selectivities TrawlSurveySel_1
selectivities_for_total_categories TrawlSurveySel_1
delta 1e-5
table obs
1995	0	0	0	0	0.000160377	0.000111474	0.000165255	0.000520272	0.000426621	0.000639776	0.00181646	0.00177729	0.00172233	0.00136643	0.00368138	0.00276982	0.00779053	0.00803412	0.00962115	0.0129738	0.0174651	0.0178496	0.0184049	0.0269278	0.0279746	0.0328895	0.0292305	0.0339443	0.0323902	0.0415441	0.0450913	0.046253	0.0402894	0.0387133	0.0312197	0.0259501	0.0158408	0.0145074	0.00889533	0.00588692	0.00343534	0.00165713	0.00131754	0.000677305	0.000468389	0.000108561	7.34E-05	0.000207115	0	0	0	0	0	0	0.000180463	0	0	4.96E-05	0.000346679	1.61E-04	0.000341624	0.000615291	0.000404958	0.000972634	0.000903046	0.00188884	0.00262415	0.00331008
1998	0	0	0	0.000262067	0.000262067	0.000149639	0.000131034	0.000499492	0.000255893	0.00146456	0.00150518	0.00129741	0.00248389	0.00358041	0.0034695	0.00285244	0.00543914	0.00449137	0.00778962	0.00765292	0.00997162	0.0141141	0.0145014	0.0126108	0.0208807	0.0181872	0.0233474	0.0273881	0.0255188	0.0355096	0.0347917	0.041853	0.0393915	0.0492512	0.0376406	0.0406878	0.0371633	0.0311177	0.0303821	0.0163014	0.0123806	0.0122209	0.00625397	0.00219066	0.00119053	0.000684215	0.000269435	0	0	0	0	0	0	0	0	0	0.000255893	0.000430365	0.000521631	0.000424591	0.000438012	0.000773583	0.000644631	0.000833868	0.00201927	0.00165061	0.00179085	0.00198072
2000	0	0	0	0	0	0	0	0	0.000322987	0.000322987	0.000322987	0.00266511	0.000369874	0.00138572	0.0023793	0.00233064	0.00228074	0.00642544	0.00202207	0.00661854	0.00596328	0.0130699	0.0151415	0.0166723	0.0117753	0.0150501	0.0183889	0.0183243	0.0251793	0.0272067	0.0302888	0.0426445	0.0344711	0.0459327	0.0546313	0.0280818	0.0233143	0.0170249	0.0120725	0.00637217	0.00672323	0.00605017	0.00237238	0.00195698	0.00361508	0.000739747	0.000369874	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0.00095329	0.00234213	0.00105311	0.00168467	0.00222957	0.00298633
2001	0	0	0	0	0.000769589	0.000537922	0	0	0.00100102	0.000391098	0.000706053	0.00317369	0.00113392	0.00311565	0.00515551	0.0040377	0.00424956	0.00983862	0.00748936	0.00877565	0.00890307	0.00779348	0.0106324	0.0143166	0.0139099	0.0165094	0.00969166	0.0289338	0.0320583	0.0265763	0.0439684	0.0519121	0.062867	0.0619169	0.030755	0.02868	0.0219375	0.0186048	0.00955954	0.00879438	0.00578588	0.00437727	0.00213456	0.00079413	0.000948447	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0
2002	0	0.000284101	0	0.000284101	0	0	0	0.00115266	0.00130284	0.00120664	0.00216694	0.00271425	0.00546171	0.00552929	0.00921337	0.00634516	0.0100126	0.0122622	0.0152832	0.0176524	0.014472	0.0188418	0.0259168	0.017065	0.0165564	0.0206997	0.0188988	0.0227266	0.0282959	0.028683	0.0255729	0.0351423	0.0317747	0.0231001	0.0273721	0.0175139	0.0125888	0.00929346	0.00618996	0.00365564	0.00345773	0.00110963	0	0	0	0.000554815	0	0	0	0	0	0	0	0	0.000284101	0	0	0.000564444	0.000284101	0.000434279	0.000889127	0.000922541	0.000294318	0.00100414	0.00224023	0.00434498	0.00498372	0.00459469
2008	0	0	0	0	0	0	0	0.000203476	0.000558084	0.000700864	0.00115911	0.00177688	0.00396144	0.00486295	0.00675586	0.00959952	0.00964329	0.0233793	0.0188347	0.0169367	0.0282501	0.0230282	0.0235084	0.0273027	0.0291135	0.028831	0.0319954	0.0224326	0.0207084	0.0224083	0.0185736	0.0193064	0.0238063	0.0313935	0.0286175	0.0248732	0.0241705	0.0154054	0.0133991	0.00810026	0.00371131	0.000962458	0.000624837	0.000969987	0.000202421	0	0	0.000212628	0	0	0	0	0	0	0	0	0	0	0.000558885	0.000406953	0.000404092	0.000558885	0	0.00149498	0.00136913	0.00302514	0.0053662	0.00855362
2012	0	0.000206524	0	0	0	0	0	0.000206524	0.000825956	0.000835762	0.00177446	0.00239804	0.00289942	0.00665712	0.00566533	0.00802241	0.00877228	0.00844899	0.00812282	0.0110754	0.0134243	0.0122425	0.0145892	0.0176558	0.0174679	0.0200321	0.0191565	0.0194462	0.0187401	0.0180736	0.0226931	0.0158082	0.0180017	0.0187405	0.0259654	0.0275044	0.0214882	0.0178625	0.0143406	0.0121681	0.00864093	0.00747305	0.0035931	0.0033777	0.00148888	0	0.000620655	0.000400628	0.00027109	0	0	0	0	0	0	0.00021619	0	0	0	0.000197313	0.00024754	0.000648073	0.000411052	0.00220535	0.00261055	0.00339202	0.00352025	0.00598232
end_table
table error_values
1995 49.2
1998 41.7
2000 43.3
2001 40.3
2002 37.4
2008 39.7
2012 36.8
end_table
)";

/**
 * Test single sex model - user supplied custom length range
 */
TEST_F(InternalEmptyLengthModel, Observation_ProportionsCategoryByLength_singlesex_bespoke_lengths) {
  AddConfigurationLine(simple_single_sex_model, __FILE__, 32);
  AddConfigurationLine(bespoke_length_range, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  obj_function.CalculateScore();
  EXPECT_NEAR(141815.92707678524, obj_function.score(), 1e-3);

  Observation* observation = model_->managers()->observation()->GetObservation("TrawlSurveyProportionAtLength");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  // these values are from CASAL the config is below
  vector<double>   expected_vals = {0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1,
                                    0.99031, 0.995799, 0.998184, 0.999216, 0.999662, 0.999854, 0.999937, 0.999973, 0.999988, 0.999995, 0.999998, 0.999999, 1, 1, 1, 1, 1,
                                    1,       1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1,        1, 1, 1, 1, 1};
  vector<unsigned> years         = {1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012};

  unsigned counter = 0;
  for (auto year_iter : comparisons) {
    for (auto comp_iter : year_iter.second) {
      EXPECT_NEAR(expected_vals[counter], comp_iter.expected_, 1e-4);
      counter++;
    }
  }
}

/**
 * Test single sex model - user doesn't supply length bins so model uses model range
 */
TEST_F(InternalEmptyLengthModel, Observation_ProportionsCategoryByLength_singlesex_default) {
  AddConfigurationLine(simple_single_sex_model, __FILE__, 32);
  AddConfigurationLine(default_length_range, __FILE__, 32);
  LoadConfiguration();

  model_->Start(RunMode::kBasic);

  ObjectiveFunction& obj_function = model_->objective_function();
  obj_function.CalculateScore();
  EXPECT_NEAR(198064.67907208874, obj_function.score(), 1e-3);

  Observation* observation = model_->managers()->observation()->GetObservation("TrawlSurveyProportionAtLength");

  map<unsigned, vector<obs::Comparison> >& comparisons = observation->comparisons();
  // The expected values should always be 0 or 1 because it's a proportion against itself as a total
  vector<double> expected_vals
      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  vector<unsigned> years = {1995, 1998, 2000, 2001, 2002, 2008, 2012};

  unsigned counter = 0;
  for (auto year_iter : comparisons) {
    for (auto comp_iter : year_iter.second) {
      EXPECT_NEAR(expected_vals[counter], comp_iter.expected_, 1e-4);
      counter++;
    }
  }
}

}  // namespace length
} /* namespace niwa */

#endif /* TESTMODE */
