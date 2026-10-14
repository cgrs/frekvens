import { mdiRouterWireless } from "@mdi/js";
import { type Component, createSignal, For } from "solid-js";

import { Icon } from "../components/Icon";
import { HOSTNAME } from "../config/constants";
import { SidebarSection } from "../extensions/WebApp";
import { WebSocketWS } from "../extensions/WebSocket";
import { MainComponent as ModesMainComponent } from "../services/Modes";

export const name = "Stream";

const ports: Record<string, number> = {
    "Art-Net": 6454,
    "Distributed Display Protocol": 4048,
    "E1.31": 5568,
};

const [getProtocol, setProtocol] = createSignal<string>("Distributed Display Protocol");
const [getProtocols, setProtocols] = createSignal<string[]>([]);

export const receiver = (json: { protocol?: string; options?: { key: string; choices?: string[] }[] }) => {
    json?.protocol !== undefined && setProtocol(json.protocol);
    const choices = json?.options?.find((option) => option.key === "protocol")?.choices;
    choices !== undefined && setProtocols(choices);
};

export const Main: Component = () => (
    <ModesMainComponent
        icon={mdiRouterWireless}
        text={`${HOSTNAME}.local:${ports[getProtocol()] ?? ""}`}
    />
);

export const Sidebar: Component = () => {
    const handleProtocol = (protocol: string) => {
        setProtocol(protocol);
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    protocol: getProtocol(),
                },
            }),
        );
    };

    return (
        <SidebarSection title="Protocol">
            <div class="action grid-cols-[--spacing(4)_1fr]">
                <Icon path={mdiRouterWireless} />
                <select
                    class="w-full"
                    value={getProtocol()}
                    onchange={(e) => handleProtocol(e.currentTarget.value)}
                >
                    <For each={getProtocols()}>{(protocol) => <option>{protocol}</option>}</For>
                </select>
            </div>
        </SidebarSection>
    );
};
