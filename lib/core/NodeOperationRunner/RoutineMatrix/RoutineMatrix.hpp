#ifndef ACOUSEA_INFRASTRUCTURE_MKR_ROUTINE_MATRIX_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_ROUTINE_MATRIX_HPP

#include "Group.hpp"

/**
 * @brief RoutineMatrix
 *        Simple alias de Group<Group<...>>.
 *        Cada Group interno tiene la misma CAPACITY.
 *        El Group externo representa el conjunto de categorías (command, response, report...).
 */
template <typename RoutineType>
using RoutineMatrix = Group<Group<RoutineType>>;


#endif // ACOUSEA_INFRASTRUCTURE_MKR_ROUTINE_MATRIX_HPP
