import QtQuick 2.15

LoginWithSSOForm {
    header.onPrevious: {
        pageLoader.setSource(Qt.resolvedUrl('qrc:/qml/HomePage.qml'))
    }

    textCode.onAccepted: {
        buttonSubmit.clicked()
    }

    buttonSwitch.onClicked: {
        loginByEmail = !loginByEmail
        textCode.text = ''
        if (loginByEmail) {
            textCode.placeholderText = qsTr('Your E-Mail')
            buttonSwitch.text = qsTr('Login by code')
        } else {
            textCode.placeholderText = qsTr('Code of your company')
            buttonSwitch.text = qsTr('Login by E-Mail')
        }
    }

    privacyCheck.onToggled: {
        isAgreePrivacyPolicy = privacyCheck.checked
        console.log("isAgreePrivacyPolicy  sso", isAgreePrivacyPolicy)
    }

    buttonSubmit.onClicked: {
        if(!privacyCheck.checked) {
            toast.show(qsTr('Please check to agree to the privacy policy and user service agreement'))
            return
        }

        if (textCode.text.toLowerCase() !== "netease") {
            toast.show(qsTr('No affiliated enterprise temporarily'))
            return
        }
        configManager.setSSOLogin(true)
        const launchUrl = composeArgumentsEx(encodeURIComponent('NEMEETING://'))
        //console.log("sso:", launchUrl)
        Qt.openUrlExternally(launchUrl)
    }

    privacyPolicy.onClicked: {
        Qt.openUrlExternally("https://reg.163.com/agreement_mobile_ysbh_wap.shtml?v=20171127")
    }

    userServiceAgreement.onClicked: {
        Qt.openUrlExternally("https://netease.im/meeting/clauses?serviceType=0")
    }

//    function composeArguments(ssoAppNamespace = '', ssoClientLoginUrl = '') {
//        let apiUrl = authManager.paasServerAddress
//        let baseUrl = apiUrl + 'v1/sso/authorize';
//        baseUrl += '?ssoAppNamespaceType='
//        baseUrl += loginByEmail ? 2 : 0
//        if (ssoAppNamespace !== '') {
//            baseUrl += '&'
//            baseUrl += 'ssoAppNamespace=' + ssoAppNamespace
//        }
//        if (ssoClientLoginUrl !== '') {
//            baseUrl += '&'
//            baseUrl += 'ssoClientLoginUrl=' + ssoClientLoginUrl
//        }

//        return baseUrl
//    }

    function composeArgumentsEx(ssoClientLoginUrl = '') {
        let apiUrl = authManager.paasServerAddress
        let baseUrl = apiUrl + 'v1/sso/authorize';
        if (ssoClientLoginUrl !== '') {
            baseUrl += '?clientCallbackUrl=' + ssoClientLoginUrl
        }

        return baseUrl
    }
}
