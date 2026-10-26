
package vn.edu.ptit.smartcane.config;

import java.nio.charset.StandardCharsets;

import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.dsl.IntegrationFlow;
import org.springframework.integration.mqtt.core.DefaultMqttPahoClientFactory;
import org.springframework.integration.mqtt.core.MqttPahoClientFactory;
import org.springframework.integration.mqtt.inbound.MqttPahoMessageDrivenChannelAdapter;
import org.springframework.integration.mqtt.support.DefaultPahoMessageConverter;
import org.springframework.integration.mqtt.support.MqttHeaders;
import org.springframework.messaging.MessageChannel;

import vn.edu.ptit.smartcane.service.MessageService;

@Configuration
public class MqttConfig {

    @Bean
    public MqttConnectOptions mqttOptions(
            @Value("${smartcane.mqtt.url}") String url,
            @Value("${smartcane.mqtt.username:}") String username,
            @Value("${smartcane.mqtt.password:}") String password) {

        MqttConnectOptions options = new MqttConnectOptions();
        options.setServerURIs(new String[]{url});
        options.setAutomaticReconnect(true);
        options.setCleanSession(true);

        if (!username.isBlank()) {
            options.setUserName(username);
            options.setPassword(password.toCharArray());
        }

        return options;
    }

    @Bean
    public MqttPahoClientFactory mqttClientFactory(
            MqttConnectOptions mqttOptions) {

        DefaultMqttPahoClientFactory factory =
                new DefaultMqttPahoClientFactory();

        factory.setConnectionOptions(mqttOptions);
        return factory;
    }

    @Bean
    public MessageChannel mqttInput() {
        return new DirectChannel();
    }

    @Bean
    public MqttPahoMessageDrivenChannelAdapter mqttInbound(
            @Value("${smartcane.mqtt.client-id}") String clientId,
            @Value("${smartcane.mqtt.topic}") String telemetry,
            @Value("${smartcane.mqtt.events-topic}") String events,
            MqttPahoClientFactory mqttClientFactory,
            MessageChannel mqttInput) {

        MqttPahoMessageDrivenChannelAdapter adapter =
                new MqttPahoMessageDrivenChannelAdapter(
                        clientId,
                        mqttClientFactory,
                        telemetry,
                        events
                );

        adapter.setQos(1);
        adapter.setConverter(new DefaultPahoMessageConverter());
        adapter.setOutputChannel(mqttInput);

        return adapter;
    }

    @Bean
    public IntegrationFlow mqttFlow(
            MessageChannel mqttInput,
            MessageService service) {

        return IntegrationFlow.from(mqttInput)
                .handle(message -> {
                    String topic = (String) message.getHeaders()
                            .get(MqttHeaders.RECEIVED_TOPIC);

                    Object raw = message.getPayload();

                    String payload = raw instanceof byte[] bytes
                            ? new String(bytes, StandardCharsets.UTF_8)
                            : String.valueOf(raw);

                    try {
                        service.process(topic, payload);
                    } catch (IllegalArgumentException exception) {
                        System.err.println(
                                "[MQTT INVALID] " + exception.getMessage()
                        );
                    }
                })
                .get();
    }
}
