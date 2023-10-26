#include <iomanip>
#include <iostream>
#include <micm/process/arrhenius_rate_constant.hpp>
#include <micm/solver/rosenbrock.hpp>

using namespace micm;

int main(const int argc, const char *argv[])
{
  auto foo = Species{ "Foo" };
  auto bar = Species{ "Bar" };
  auto baz = Species{ "Baz" };

  Phase gas_phase{ std::vector<Species>{ foo, bar, baz } };

  System chemical_system{ SystemParameters{ .gas_phase_ = gas_phase } };

  Process r1 = Process::create()
                   .reactants({ foo })
                   .products({ Yield(bar, 0.8), Yield(baz, 0.2) })
                   .rate_constant(ArrheniusRateConstant({ .A_ = 1.0e-3 }))
                   .phase(gas_phase);

  Process r2 = Process::create()
                   .reactants({ foo, bar })
                   .products({ Yield(baz, 1) })
                   .rate_constant(ArrheniusRateConstant({ .A_ = 1.0e-5, .C_ = 110.0 }))
                   .phase(gas_phase);

  std::vector<Process> reactions{ r1, r2 };

  RosenbrockSolver<> solver{ chemical_system, reactions, RosenbrockSolverParameters::three_stage_rosenbrock_parameters() };

  State state = solver.GetState();

  state.conditions_[0].temperature_ = 287.45;  // K
  state.conditions_[0].pressure_ = 101319.9;   // Pa
  state.SetConcentration(foo, 20.0);           // mol m-3

  std::cout << std::setw(5) << "time [s]," << std::setw(13) << "foo, " << std::setw(12) << "bar, " << std::setw(10) << "baz"
            << std::endl;
  for (int i = 0; i < 10; ++i)
  {
    auto result = solver.Solve(500.0, state);
    state.variables_ = result.result_;
    std::cout << std::setfill(' ') << std::fixed << std::setw(8) << std::setprecision(2) << i * 500.0 << ", "
              << std::setw(10) << std::setprecision(4) << state.variables_[0][state.variable_map_["Foo"]] << ", "
              << std::setw(10) << std::setprecision(4) << state.variables_[0][state.variable_map_["Bar"]] << ", "
              << std::setw(10) << std::setprecision(4) << state.variables_[0][state.variable_map_["Baz"]] << std::endl;
  }

  return 0;
}