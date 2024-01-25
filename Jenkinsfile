@Library('xmos_jenkins_shared_library@v0.27.0') _

// Get XCommon CMake and log a record of the git commit
def get_xcommon_cmake() {
  sh "git clone -b develop git@github.com:xmos/xcommon_cmake"
  sh "git -C xcommon_cmake rev-parse HEAD"
}

getApproval()

pipeline {
  agent none
  options {
    skipDefaultCheckout()
    timestamps()
    buildDiscarder(xmosDiscardBuildSettings(onlyArtifacts=true))
  }
  parameters {
      choice(name: 'TEST_LEVEL', choices: ['smoke', 'nightly', 'weekend'],
             description: 'The level of test coverage to run')
  }
  environment {
    REPO = 'sw_usb_audio'
    VIEW = getViewName(REPO)
    TOOLS_VERSION = "15.2.1"
    XTAGCTL_VERSION = "v2.0.0"
  }
  stages {
    stage('Build') {
      steps {
        echo "Pass"
      }
    }  // Regression Test
  }
}
