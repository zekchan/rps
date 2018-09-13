import nodeInterfaceFabric from "./tools/nodeInterfaceFabric";

const nodeInterface = nodeInterfaceFabric('https://api.kylin-testnet.eospace.io')
const eos = nodeInterface.eos;
export default eos;
export const cleos = nodeInterface.cleos;
