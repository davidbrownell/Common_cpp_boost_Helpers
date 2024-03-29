# ----------------------------------------------------------------------
# |
# |  Setup_custom.py
# |
# |  David Brownell <db@DavidBrownell.com>
# |      2018-05-03 22:12:13
# |
# ----------------------------------------------------------------------
# |
# |  Copyright David Brownell 2018-22.
# |  Distributed under the Boost Software License, Version 1.0.
# |  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
# |
# ----------------------------------------------------------------------
"""Performs repository-specific setup activities."""

# ----------------------------------------------------------------------
# |
# |  To setup an environment, run:
# |
# |     Setup(.cmd|.ps1|.sh) [/debug] [/verbose] [/configuration=<config_name>]*
# |
# ----------------------------------------------------------------------

import os
import sys

from collections import OrderedDict

import CommonEnvironment

# ----------------------------------------------------------------------
_script_fullpath                            = CommonEnvironment.ThisFullpath()
_script_dir, _script_name                   = os.path.split(_script_fullpath)
# ----------------------------------------------------------------------

# <Missing function docstring> pylint: disable = C0111
# <Line too long> pylint: disable = C0301
# <Wrong hanging indentation> pylint: disable = C0330
# <Class '<name>' has no '<attr>' member> pylint: disable = E1103
# <Unreachable code> pylint: disable = W0101
# <Wildcard import> pylint: disable = W0401
# <Unused argument> pylint: disable = W0613

fundamental_repo                                                            = os.getenv("DEVELOPMENT_ENVIRONMENT_FUNDAMENTAL")
assert os.path.isdir(fundamental_repo), fundamental_repo

sys.path.insert(0, fundamental_repo)
from RepositoryBootstrap import *                                           # <Unused import> pylint: disable = W0614
from RepositoryBootstrap.SetupAndActivate import CurrentShell               # <Unused import> pylint: disable = W0614
from RepositoryBootstrap.SetupAndActivate.Configuration import *            # <Unused import> pylint: disable = W0614

del sys.path[0]

# ----------------------------------------------------------------------
# There are two types of repositories: Standard and Mixin. Only one standard
# repository may be activated within an environment at a time while any number
# of mixin repositories can be activated within a standard repository environment.
# Standard repositories may be dependent on other repositories (thereby inheriting
# their functionality), support multiple configurations, and specify version
# information for tools and libraries in themselves or its dependencies.
#
# Mixin repositories are designed to augment other repositories. They cannot
# have configurations or dependencies and may not be activated on their own.
#
# These difference are summarized in this table:
#
#                                                       Standard  Mixin
#                                                       --------  -----
#      Can be activated in isolation                       X
#      Supports configurations                             X
#      Supports VersionSpecs                               X
#      Can be dependent upon other repositories            X
#      Can be activated within any other Standard                  X
#        repository
#
# Consider a script that wraps common Git commands. This functionality is useful
# across a number of different repositories, yet doesn't have functionality that
# is useful on its own; it provides functionality that augments other repositories.
# This functionality should be included within a repository that is classified
# as a mixin repository.
#
# To classify a repository as a Mixin repository, decorate the GetDependencies method
# with the MixinRepository decorator.
#


# @MixinRepository # <-- Uncomment this line to classify this repository as a mixin repository
def GetDependencies():
    """
    Returns information about the dependencies required by this repository.

    The return value should be an OrderedDict if the repository supports multiple configurations
    (aka is configurable) or a single Configuration if not.
    """

    if CurrentShell.CategoryName == "Windows":
        architectures = ["x64", "x86"]

        compiler_infos = [
            ("MSVC_2019", None),
            # ("Clang_8", None),
            ("Clang_10", "_ex"),
        ]

    else:
        # Cross compiling on Linux is much more difficult on Linux than it is on
        # Windows. Only support the current architecture.
        architectures = [CurrentShell.Architecture]

        compiler_infos = [
            # ("Clang_8", None),
            ("Clang_10", "_ex"),
        ]

    d = OrderedDict()

    for boost_version in ["1.70.0"]:
        d[boost_version] = Configuration(
            "boost {} (No compiler dependencies)".format(boost_version),
            [
                Dependency(
                    "407DD743110A4FB1871AEF60CBEC99A0",
                    "Common_cpp_boost_{}".format(boost_version),
                    "standard",
                    "https://github.com/davidbrownell/Common_cpp_boost_{}.git".format(boost_version),
                ),
                Dependency(
                    "398F6BEC057C4FE4B724153DF4EB8AE4",
                    "Common_cpp_Helpers",
                    "standard",
                    "https://github.com/davidbrownell/Common_cpp_Helpers.git",
                ),
            ],
        )

        for config_name, architecture_configuration_suffix in compiler_infos:
            architecture_configuration_suffix = architecture_configuration_suffix or ""

            for architecture in architectures:
                this_config_name = "{}_{}_{}{}".format(boost_version, config_name, architecture, architecture_configuration_suffix)
                this_config_desc = "boost {} - {} ({}{})".format(boost_version, config_name, architecture, architecture_configuration_suffix)

                d[this_config_name] = Configuration(
                    this_config_desc,
                    [
                        Dependency(
                            "407DD743110A4FB1871AEF60CBEC99A0",
                            "Common_cpp_boost_{}".format(boost_version),
                            "{}_{}{}".format(config_name, architecture, architecture_configuration_suffix),
                            "https://github.com/davidbrownell/Common_cpp_boost_{}.git".format(boost_version),
                        ),
                        Dependency(
                            "398F6BEC057C4FE4B724153DF4EB8AE4",
                            "Common_cpp_Helpers",
                            "standard",
                            "https://github.com/davidbrownell/Common_cpp_Helpers.git",
                        ),
                    ],
                )

    return d


# ----------------------------------------------------------------------
def GetCustomActions(debug, verbose, explicit_configurations):
    """
    Returns an action or list of actions that should be invoked as part of the setup process.

    Actions are generic command line statements defined in
    <Common_Environment>/Libraries/Python/CommonEnvironment/v1.0/CommonEnvironment/Shell/Commands/__init__.py
    that are converted into statements appropriate for the current scripting language (in most
    cases, this is Bash on Linux systems and Batch or PowerShell on Windows systems.
    """

    return []
